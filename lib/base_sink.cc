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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <bokehgui/base_sink.h>
#include <volk/volk.h>

namespace gr {
  namespace bokehgui {
    template <class T, class U>
    base_sink<T, U>::base_sink(std::string class_name, int size, const std::string &name, int nconnections)
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

    template <class T, class U>
    base_sink<T, U>::~base_sink()
    {
      while(!d_buffers.empty())
        d_buffers.pop();
    }

    template <class T, class U>
    void
    base_sink<T, U>::get_plot_data(U** output_items, int* nrows, int* size) {
      gr::thread::scoped_lock lock(d_setlock);
      if (!d_buffers.size()) {
        *size = 0;
        *nrows = d_nconnections + 1;
        return;
      }

      *nrows = d_nconnections + 1;
      *size = d_buffers.front()[0].size();

      U* arr = (U*)volk_malloc((*nrows)*(*size)*sizeof(T), volk_get_alignment());

      process_plot(arr, *nrows, *size);

      *output_items = arr;

      d_buffers.pop();

      return;
    }

    template <class T, class U>
    int
    base_sink<T, U>::work (int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items) {
      gr::thread::scoped_lock lock(d_setlock);
      std::cout << "dfd" << std::endl;
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
      std::cout << "dfd1" << std::endl;
            pop_other_queues();
          }

          std::vector<std::vector<T> > data_buff;
          data_buff.reserve(d_nconnections + 1);
          d_buffers.push(data_buff);
      std::cout << "dfd" << std::endl;

          for(int n = 0; n < d_nconnections + 1; n++) {
            d_buffers.back().push_back(std::vector<T>(nitems, 0));
            if (n == d_nconnections) {
              continue;
            }
            in = (const T*) input_items[n];
      std::cout << "dfd" << std::endl;
            memcpy(&d_buffers.back()[n][0], &in[d_index + 1], nitems*sizeof(T));
          }
      std::cout << "dfd" << std::endl;

          work_process_other_queues(d_index, nitems);
      std::cout << "dfd" << std::endl;

          if(d_trigger_mode != TRIG_MODE_FREE)
            d_triggered = false;
        }
      std::cout << "dfd" << std::endl;
        d_index += nitems;
      }
      return noutput_items;
    }

    template <class T, class U>
    bool
    base_sink<T, U>::check_topology(int ninputs, int noutputs) {
      return ninputs == d_nconnections;
    }

    template <class T, class U>
    void
    base_sink<T, U>::clear_queue() {
      while(!d_buffers.empty())
        d_buffers.pop();
    }

    template <class T, class U>
    void
    base_sink<T, U>::handle_pdus(pmt::pmt_t msg) {
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
        throw std::runtime_error(d_name + ": message must be either"
                                 "a PDU or a uniform vector of samples.");
      }

      len = pmt::length(samples);

      const T *in;
      verify_datatype_PDU(in, samples, len);

      // Copy data to buffer
      set_size(len);
      if(d_buffers.size() == d_queue_size)
        d_buffers.pop();

      std::vector<std::vector<T> > data_buff;
      data_buff.reserve(d_nconnections + 1);
      d_buffers.push(data_buff);

      for(int n = 0; n < d_nconnections + 1; n++) {
        d_buffers.back().push_back(std::vector<T> (len, 0));
      }
      memcpy(&d_buffers.back()[d_nconnections][0], in, len*sizeof(T));
    }

    template <class T, class U>
    int
    base_sink<T, U>::get_size() {
      return d_size;
    }

    template <class T, class U>
    std::string
    base_sink<T, U>::get_name()
    {
      return d_name;
    }

    template <class T, class U>
    int
    base_sink<T, U>::get_nconnections()
    {
      return d_nconnections;
    }

    template class base_sink<float, float>;

  } /* namespace bokehgui */
} /* namespace gr */
