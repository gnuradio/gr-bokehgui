/* -*- c++ -*- */
/*
 * Copyright 2008-2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_BOKEHGUI_BASE_SINK_PROC_H
#define INCLUDED_BOKEHGUI_BASE_SINK_PROC_H

#include <queue>
#include <bokehgui/api.h>
#include <gnuradio/sync_block.h>
#include <bokehgui/trigger_mode.h>
#include <volk/volk.h>

namespace gr {
  namespace bokehgui {

    /*!
     * \brief A Template class for all bokeh based sinks. Primarily manages Buffer operations
     * \ingroup bokehgui
     *
     * \details
     * The instantiation of this template class is a parent class to the
     * store-sinks in this module. The two classes in this template are
     * used as follows:
     * class T -> Type of input stream into the block.
     * class U -> Type of output via get_plot_data.
     *
     * This class also defines set of virtual functions that must be defined
     * by the child-class. If the functions are irrelevant for the child-class
     * then implementation of a blank function is required.
     *
     * This class maintains a queue of 2D array is maintained. Each 2D array
     * is of size \p nconnection \p x \p d_size. For each call to get the
     * data from Python, first element of queue is sent.
     *
     * The base_class also supports storing appropriate type  data or
     * messages. The message port is named "in". When using message port,
     * \p nconnections should be set to 0.
     *
     * This class implements its functions in header file only because of
     * limitation provided by SWIG for template classes.
     */

    template <class T, class U> class base_sink : public sync_block
    {
     public:
      base_sink() {} // To allow a virtual inheritance
      base_sink(std::string class_name, int size, const std::string &name, int nconnections)
        : sync_block(class_name,
                     io_signature::make(0, nconnections, sizeof(T)),
                     io_signature::make(0, 0, 0)),
        d_size(size), d_name(name), d_nconnections(nconnections)
      {
        d_queue_size = BOKEH_BUFFER_QUEUE_SIZE;
        d_index = 0;

        message_port_register_in(pmt::mp("in"));
        set_msg_handler(pmt::mp("in"),
                        boost::bind(&base_sink<T, U>::handle_pdus, this, _1));

        const int alignment_multiple = volk_get_alignment() / sizeof(T);
        set_alignment(std::max(1, alignment_multiple));

        set_output_multiple (d_size);
      }
      ~base_sink() {
        while(!d_buffers.empty())
          d_buffers.pop();
      }

      /*!
       * \brief Called from Python to get the first element of queue.
       *
       * The function takes no argument when called from Python. Using
       * a beautiful combination of SWIG and Numpy, the 2D array \p output_items
       * is returned to Python interface.
       *
       * After each call, first element of Queue is popped to allow further
       * data to be stored.
       *
       * \param output_items Pointer to 2D array to be sent to Python
       * \param nrows Pointer to integer value representing number of rows
       *              Generally, \p nconnection+1
       * \param size Pointer to integer value representing number of elements
       *             in a row. Generally \p d_size
       */
      void get_plot_data (U** output_items, int* nrows, int* size) {
        gr::thread::scoped_lock lock(d_setlock);
        if (!d_buffers.size()) {
          *size = 0;
          *nrows = d_nconnections + 1;
          return;
        }

        *nrows = d_nconnections + 1;
        *size = d_buffers.front()[0].size();

        U* arr = (U*) malloc(2*(*nrows)*(*size)*sizeof(U));
        memset(arr, 0, 2*(*nrows)*(*size)*sizeof(U));
        std::cout << "Should be 0" <<std::endl;
        for(int n = 0; n < *nrows; n++) {
          for(int i = 0; i < *size; i++) {
            std::cout << arr[n*(*size) + i] << " ";
          }
          std::cout << std::endl;
        }
        process_plot(arr, nrows, size);
        std::cout << "Finally" << std::endl;
        for(int n = 0; n < *nrows; n++) {
          for(int i = 0; i < *size; i++) {
            std::cout << arr[n*(*size) + i] << " ";
          }
          std::cout << std::endl;
        }

        *output_items = arr;

        d_buffers.pop();

        return;
      }

      int work(int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items) {
        gr::thread::scoped_lock lock(d_setlock);
        const T *in;
        // Consume all possible set of data. Each with nitems
        for(int d_index = 0; d_index < noutput_items;) {
          int nitems = std::min(d_size, noutput_items - d_index);
          // TODO: See how to include auto/normal triggeres
          if(d_trigger_mode == TRIG_MODE_TAG) {
            _test_trigger_tags(d_index, nitems);
          }
          // TODO: else normal trigger

          // The d_triggered and d_index is set.
          // We will now check if triggered.
          // If it is triggered then we check the trigger_index = d_index
          // We will start looking from d_index to the end of input_items
          // First check if d_index+d_size < noutput_items
          if(d_triggered) {
            if (d_index + nitems > noutput_items) {
              nitems = noutput_items - d_index;
            }
            if(d_buffers.size() == d_queue_size) {
              d_buffers.pop();
              pop_other_queues();
            }

            std::vector<std::vector<T> > data_buff;
            data_buff.reserve(d_nconnections + 1);
            d_buffers.push(data_buff);

            for(int n = 0; n < d_nconnections + 1; n++) {
              d_buffers.back().push_back(std::vector<T>(nitems, 0));
              if (n == d_nconnections) {
                continue;
              }
              in = (const T*) input_items[n];
              memcpy(&d_buffers.back()[n][0], &in[d_index], nitems*sizeof(T));
            }

            work_process_other_queues(d_index, nitems);
            if(d_trigger_mode != TRIG_MODE_FREE)
              d_triggered = false;
          }
          d_index += nitems;
        }
        return noutput_items;
      }

      void handle_pdus(pmt::pmt_t msg) {
        size_t len;
        pmt::pmt_t dict, samples;

        // Test to make sure this is either PDU or Uniform vector of
        // samples. Get the samples PMT and the dictionar if it's a PDU.
        // If not, we throw an error and exit.
        if(pmt::is_pair(msg)) {
          dict = pmt::car(msg);
          samples = pmt::cdr(msg);
        }
        else if(pmt::is_uniform_vector(msg)) {
          samples = msg;
        }
        else {
          throw std::runtime_error(d_name + ": message must be either "
                                   "a PDU or a uniform vector of samples.");
        }

        len = pmt::length(samples);

        const T *in;
        gr::thread::scoped_lock lock(d_setlock);
        verify_datatype_PDU(in, samples, len);
        d_len.push(len);
        // Copy data to buffer
        if(d_buffers.size() == d_queue_size)
          d_buffers.pop();

        std::vector<std::vector<T> > data_buff;
        data_buff.reserve(d_nconnections + 1);
        d_buffers.push(data_buff);

        for(int n = 0; n < d_nconnections + 1; n++) {
          d_buffers.back().push_back(std::vector<T> (d_size, 0));
        }
        memcpy(&d_buffers.back()[d_nconnections][0], in, d_size*sizeof(T));
      }

      bool check_topology(int ninputs, int noutputs) {
        return ninputs == d_nconnections;
      }
      void clear_queue() {
          while(!d_buffers.empty())
            d_buffers.pop();
      }
      int get_size() {
        return d_size;
      }
      std::string get_name() {
        return d_name;
      }
      int get_nconnections() {
        return d_nconnections;
      }
      virtual void _test_trigger_tags(int, int) = 0;
      virtual void pop_other_queues() = 0;
      virtual void verify_datatype_PDU(const T*, pmt::pmt_t, size_t) = 0;
      virtual void process_plot(U* arr, int* nrows, int* size) = 0;
      virtual void work_process_other_queues(int, int) = 0;
      virtual void set_size(int) = 0;
     protected:
      // 2D array of size (nconn*nitems). The int represents nitems
      std::queue<std::vector<std::vector<T> > > d_buffers;
      // Size of plot
      int d_size;
      // Name of the block and Title of the plot
      std::string d_name;
      // Number of connections
      int d_nconnections;
      // Used during checking a trigger
      // Use output from this index
      int d_index;
      int d_queue_size;

      // Only used for handle pdu. saves the PDU length corresponding to each item in d_buffers queue.
      // Will be used in process_plot_data
      std::queue<float> d_len;

      // Members used for triggering scope
      bool d_triggered;
      trigger_mode d_trigger_mode;
      int d_trigger_channel;
      pmt::pmt_t d_trigger_tag_key;
    };
  }
}

#endif /* INCLUDED_BOKEHGUI_BASE_SINK_PROC_H */
