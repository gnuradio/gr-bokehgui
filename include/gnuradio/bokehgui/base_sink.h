/* -*- c++ -*- */
/*
 * Copyright 2017 Free Software Foundation, Inc.
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
#include <gnuradio/bokehgui/api.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/bokehgui/trigger_mode.h>
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
     *
     * This class also defines set of virtual functions that must be defined
     * by the child-class. If the functions are irrelevant for the child-class
     * then implementation of a blank function is required.
     *
     * This class maintains a queue of 2D arrays. Each 2D array
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

    template <class T> class base_sink : public sync_block
    {
     public:
      base_sink() {} // To allow a virtual inheritance
      base_sink(std::string class_name, int size, const std::string &name, int nconnections, unsigned int vlen = 1)
        : sync_block(class_name,
                     io_signature::make(0, nconnections, sizeof(T)*vlen),
                     io_signature::make(0, 0, 0)),
        d_size(size), d_name(name), d_nconnections(nconnections), d_vlen(vlen)
      {
        d_queue_size = BOKEH_BUFFER_QUEUE_SIZE;
        d_index = 0;

        message_port_register_in(pmt::mp("in"));
        set_msg_handler(pmt::mp("in"), [this](pmt::pmt_t msg) { this->handle_pdus(msg); });

        const int alignment_multiple = volk_get_alignment() / (sizeof(T)*d_vlen);
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
      float * get_plot_data () {
        gr::thread::scoped_lock lock(d_setlock);
        if (!d_buffers.size()) {
          int size = 0;
          int nrows = d_nconnections + 1;
          float* arr = (float*) malloc(2*(nrows)*(size)*sizeof(float));
          return arr;
        }

        int nrows = d_nconnections + 1;  //Why the +1? why not d_buffers.front().size()?
        int size = d_buffers.front()[0].size();

        float* arr = (float*) malloc(2*(nrows)*(size)*sizeof(float)); // Why the 2* and the size float and not T?
        memset(arr, 0, 2*(nrows)*(size)*sizeof(float));
        process_plot(arr, &nrows, &size);

        d_buffers.pop();

        return arr;
      }

      int get_buff_size(){
        gr::thread::scoped_lock lock(d_setlock);
        if (!d_buffers.size()) {
          // printf("The buffer is empty, returning 0\n");
          return 0;
        }
        return d_buffers.front()[0].size();
      }

      int get_buff_num_items(){
        return d_buffers.size();
      }

      int get_buff_cols(){
        return (sizeof(T)/sizeof(float))*d_nconnections;
      }

      int work(int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items) {
        gr::thread::scoped_lock lock(d_setlock);
        const T *in;
        // Consume all possible set of data. Each with nitems
        for(d_index = 0; d_index < noutput_items;) {
          int nitems = std::min(d_size, noutput_items - d_index);
          // TODO: See how to include auto/normal triggers
          if(d_trigger_mode == TRIG_MODE_TAG) {
            _test_trigger_tags(d_index, nitems);
          }
          // TODO: else normal trigger
          else if (d_trigger_mode != TRIG_MODE_FREE)
          {
            _test_trigger_norm(d_index, nitems, input_items);
          }

          // The d_triggered and d_index is set.
          // We will now check if triggered.
          // If it is triggered then we check the trigger_index = d_index
          // We will start looking from d_index to the end of input_items
          // First check if d_index+d_size < noutput_items
          if(d_triggered) {
            if (d_index + d_size > noutput_items) { // This should never happen, it's handled above, except if trigger active, with delay
              // So we don't consume the corresponding items, only up to the new index, since that would lead to undersized buffers
              d_triggered = false;
              return d_index;
            }
            nitems = d_size; // We've remove the edge case with buffer not full enough
            if(d_buffers.size() == d_queue_size) {  //Make room if buffer queue is full
              d_buffers.pop();
              pop_other_queues();
            }

            std::vector<std::vector<T> > data_buff;
            data_buff.reserve(d_nconnections + 1);
            d_buffers.push(data_buff);

            for(int n = 0; n < d_nconnections + 1; n++) { // Why the +1?
              d_buffers.back().push_back(std::vector<T>(nitems*d_vlen, 0));
              if (n == d_nconnections) {
                continue;
              }
              in = (const T*) input_items[n];
              memcpy(&d_buffers.back()[n][0], &in[d_index*d_vlen], nitems*sizeof(T)*d_vlen);
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
      int get_vlen() {
        return d_vlen;
      }
      std::string get_name() {
        return d_name;
      }
      int get_nconnections() {
        return d_nconnections;
      }
      virtual void _test_trigger_tags(int, int) = 0;
      virtual void _test_trigger_norm(int, int, gr_vector_const_void_star) = 0;
      virtual void pop_other_queues() = 0;
      virtual void verify_datatype_PDU(const T*, pmt::pmt_t, size_t) = 0;
      virtual void process_plot(float* arr, int* nrows, int* size) = 0;
      virtual void work_process_other_queues(int, int) = 0;
      virtual void set_size(int) = 0;
     protected:
      // 2D array of size (nconn*nitems). The int represents nitems
      std::queue<std::vector<std::vector<T> > > d_buffers;
      // Size of plot
      int d_size;
      unsigned int d_vlen;
      // Name of the block and Title of the plot
      std::string d_name;
      // Number of connections
      int d_nconnections;
      // Used during checking a trigger
      // Use output from this index
      int d_index;
      int d_queue_size;

      // Only used for handle pdu.
      // Saves the PDU length corresponding to each item
      // in d_buffers queue when using message handling
      // It will be used in process_plot_data for message port
      std::queue<float> d_len;

      // Members used for scope triggers
      bool d_triggered;
      trigger_mode d_trigger_mode;
      int d_trigger_channel;
      pmt::pmt_t d_trigger_tag_key;
    };
  }
}

#endif /* INCLUDED_BOKEHGUI_BASE_SINK_PROC_H */
