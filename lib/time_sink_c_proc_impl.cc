/* -*- c++ -*- */
/* Copyright 2011-2013,2015 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
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
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <gnuradio/block_detail.h>
#include <volk/volk.h>
#include "time_sink_c_proc_impl.h"

namespace gr {
  namespace bokehgui {

    time_sink_c_proc::sptr
    time_sink_c_proc::make(int size, double sample_rate, const std::string &name, int nconnections)
    {
      return gnuradio::get_initial_sptr
        (new time_sink_c_proc_impl(size, sample_rate, name, nconnections));
    }

    time_sink_c_proc_impl::time_sink_c_proc_impl(int size, double samp_rate, const std::string &name, int nconnections)
      : gr::sync_block("time_sink_c_proc",
              gr::io_signature::make(0, nconnections, sizeof(gr_complex)),
              gr::io_signature::make(0, 0, 0)),
      d_size(size), d_buffer_size(3*size), d_samp_rate(samp_rate), d_name(name),
      d_nconnections(nconnections)
    {
      // setup PDU handling input port
      message_port_register_in(pmt::mp("in"));
      set_msg_handler(pmt::mp("in"),
                      boost::bind(&time_sink_c_proc_impl::handle_pdus, this, _1));

      // define float buffer with PDU message handling
      // Even integers -> I - part of signal
      // Odd integers -> Q - part of signal
      for(int n = 0; n < 2*d_nconnections+2; n++) {
        d_buffers.push_back((float*)volk_malloc(d_buffer_size*sizeof(float),

                  volk_get_alignment()));
        memset(d_buffers[n], 0, d_buffer_size*sizeof(float));
      }

      d_xbuffers = (float*)volk_malloc(d_size*sizeof(float), volk_get_alignment());
      for (int i = 0; i < d_size; i++) {
        d_xbuffers[i] = i/samp_rate;
      }

      // Set alignment properties for VOLK
      const int alignment_multiple = volk_get_alignment() / sizeof(float);
      set_alignment(std::max(1, alignment_multiple));

      d_tags = std::vector<std::vector<gr::tag_t> >(d_nconnections);

      set_output_multiple(d_size);

      d_start = 0;
      d_end = d_buffer_size;
      d_index = 0;

      // set_trigger_mode(static_cast<int>(TRIG_MODE_FREE), static_cast<int>(TRIG_SLOPE_POS), 0.0, 0.0, 0, "");

      set_history(2);          // so we can look ahead for the trigger slope
      declare_sample_delay(1); // delay the tags for a history of 2
    }

    time_sink_c_proc_impl::~time_sink_c_proc_impl()
    {
      for(int n = 0; n < d_nconnections; n++) {
        volk_free(d_buffers[n]);
      }
      volk_free(d_xbuffers);
    }

    bool
    time_sink_c_proc_impl::check_topology(int ninputs, int noutputs)
    {
      return ninputs == d_nconnections;
    }

    void
    time_sink_c_proc_impl::get_plot_data(float** output_items, int* nrows, int* size) {
      gr::thread::scoped_lock lock(d_setlock);
      if (d_index < d_size)
        *size = 0;
      else
        *size = d_size;
      // 0th row for xbuffer
      // (2*i+1)th row for I-part
      // (2*i+2)th row for Q-part of i-th input; 0 <= i < d_nconnections;
      // (2*d_nconnections + 1)th row for I-part of PDU message
      // (2*d_nconnections + 2)th row for Q-part of PDU message
      *nrows = 2*d_nconnections + 3;
      float* arr = (float*)volk_malloc((*nrows)*(*size)*sizeof(float), volk_get_alignment());

      for(int i = 0; i < *size; i++) {
        arr[i] = d_xbuffers[i];
      }

      for(int n = 1; n < *nrows; n++) {
        for(int i = 0; i < *size; i++) {
          arr[n*(*size)+i] = d_buffers[n-1][i];
        }
      }
      *output_items = arr;
      if(*size) {
        for (int n = 0; n < 2*d_nconnections+2; n++) {
          memmove(&d_buffers[n][0], &d_buffers[n][*size], (d_end-(*size))*sizeof(float));
        }
      }
      d_index -= *size;
      return;
    }

    int
    time_sink_c_proc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      int n = 0;
      const gr_complex *in;

      int nfill = d_end - d_index; // Room left in buffers
      int nitems = std::min(nfill, noutput_items);
      for(int n = 0; n < d_nconnections; n++) {
        in = (const gr_complex*) input_items[n];
        volk_32fc_deinterleave_32f_x2(d_buffers[2*n+0], d_buffers[2*n+1],
                                      &in[1], nitems);

        uint64_t nr = nitems_read(n);
        std::vector<gr::tag_t> tags;
        get_tags_in_range(tags, n, nr, nr + nitems);
        for(size_t t = 0; t < tags.size(); t++) {
          tags[t].offset = tags[t].offset - nr + (d_index-d_start-1);
        }
        d_tags[n].insert(d_tags[n].end(), tags.begin(), tags.end());
      }
      d_index += nitems;
      return nitems;
    }

    void
    time_sink_c_proc_impl::handle_pdus(pmt::pmt_t tags) {

    }

  } /* namespace bokehgui */
} /* namespace gr */

