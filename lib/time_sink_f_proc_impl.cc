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
#include <volk/volk.h>
#include "time_sink_f_proc_impl.h"

namespace gr {
  namespace bokehgui {

    time_sink_f_proc::sptr
    time_sink_f_proc::make(int size, double samp_rate, const std::string &name, int nconnections)
    {
      return gnuradio::get_initial_sptr
        (new time_sink_f_proc_impl(size, samp_rate, name, nconnections));
    }

    /*
     * The private constructor
     */
    time_sink_f_proc_impl::time_sink_f_proc_impl(int size, double samp_rate, const std::string &name, int nconnections)
      : sync_block("time_sink_f_proc",
                   io_signature::make(0, nconnections, sizeof(float)),
                   io_signature::make(0, 0, 0)),
        d_size(size), d_buffer_size(3*size), d_samp_rate(samp_rate), d_name(name),
        d_nconnections(nconnections)
    {
      for (int n = 0; n < d_nconnections; n++) {
        d_buffers.push_back((float*)volk_malloc(d_buffer_size*sizeof(float), volk_get_alignment()));
        memset(d_buffers[n], 0, d_buffer_size*sizeof(float));
      }
      d_xbuffers = (float*)volk_malloc(d_size*sizeof(float), volk_get_alignment());
      for (int i = 0; i < d_size; i++) {
        d_xbuffers[i] = i/samp_rate;
      }
      const int alignment_multiple = volk_get_alignment() / sizeof(float);
      set_alignment(std::max(1, alignment_multiple));

      set_output_multiple(d_size);

      d_start = 0;
      d_end = d_buffer_size;
      d_index = 0;
    }

    /*
     * Our virtual destructor.
     */
    time_sink_f_proc_impl::~time_sink_f_proc_impl()
    {
      for(int n = 0; n < d_nconnections; n++) {
        volk_free(d_buffers[n]);
      }
      volk_free(d_xbuffers);
    }

    void
    time_sink_f_proc_impl::get_plot_data(float** output_items, int* nrows, int* size) {
      gr::thread::scoped_lock lock(d_setlock);
      *size = d_size;
      *nrows = d_nconnections + 1;
      float* arr = (float*)volk_malloc((*nrows)*(*size)*sizeof(float), volk_get_alignment());

      for (int n=0; n < *nrows; n++) {
        for (int i = 0; i < *size; i++) {
          if (n == 0)
            arr[n*(*size)+i] = d_xbuffers[i];
          else
            arr[n*(*size)+i] = d_buffers[n-1][i];
        }
      }
      *output_items = arr;
      for (int n=0; n < d_nconnections; n++) {
        memmove(&d_buffers[n][0], &d_buffers[n][d_size], (d_end - d_size)*sizeof(float));
      }
      d_index -= d_size;
      return;
    }

    int time_sink_f_proc_impl::work (int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items) {
      gr::thread::scoped_lock lock(d_setlock);
      int n=0;
      const float *in;

      if (noutput_items > d_buffer_size) {
        int dropped = noutput_items - d_buffer_size;
        for (n=0; n<d_nconnections; n++) {
          in = (const float*) input_items[n];
          memcpy(&d_buffers[n][0], &in[dropped], d_buffer_size*sizeof(float));
        }
        d_index = d_end;
        return noutput_items;
      }
      int nfill = d_end - d_index; // Room left in buffers
      if (nfill >= noutput_items) { // If enough room left, store the values
        for (n=0; n<d_nconnections; n++) {
          in = (const float*) input_items[n];
          memmove(&d_buffers[n][d_index], &in[n], noutput_items*sizeof(float));
        }
      }
      else { // If not enough room,
        int overflow = noutput_items - nfill;
        for (n=0; n<d_nconnections; n++) {
          in = (const float*) input_items[n];
          // Then shift the buffer by overflow length
          memmove(&d_buffers[n][0], &d_buffers[n][overflow], d_index*sizeof(float));
          d_index -= overflow;
          // Then copy the buffer in remaining length
          memcpy(&d_buffers[n][d_index], &in[n], noutput_items*sizeof(float));
        }
      }
      d_index += noutput_items;
      return noutput_items;
    }

    bool
    time_sink_f_proc_impl::check_topology(int ninputs, int noutputs)
    {
      return ninputs == d_nconnections;
    }

    void
    time_sink_f_proc_impl::set_nsamps(const int newsize)
    {
      if(newsize != d_size) {
        gr::thread::scoped_lock lock(d_setlock);
        // Set new size and rest buffer indexs.
        // Throws away current data!
        d_size = newsize;
        d_buffer_size = 3*d_size;

        // Resize buffers and relapce data
        volk_free(d_xbuffers);
        d_xbuffers = (float*)volk_malloc(d_buffer_size*sizeof(float), volk_get_alignment());
        for (int i = 0; i < d_size; i++)
          d_xbuffers[i] = i/d_samp_rate;
        for (int n = 0; n < d_nconnections; n++) {
      	  volk_free(d_buffers[n]);
      	  d_buffers[n] = (float*)volk_malloc(d_buffer_size*sizeof(float),
                                                    volk_get_alignment());
      	  memset(d_buffers[n], 0, d_buffer_size*sizeof(float));
        }
        d_start = 0;
        d_end = d_buffer_size;
        d_index = 0;
      }
    }

    void
    time_sink_f_proc_impl::set_samp_rate(const double samp_rate)
    {
      gr::thread::scoped_lock lock(d_setlock);
      d_samp_rate = samp_rate;
      for (int i = 0; i < d_size; i++)
        d_xbuffers[i] = i/d_samp_rate;
    }

    int
    time_sink_f_proc_impl::nsamps() const
    {
      return d_size;
    }

    void
    time_sink_f_proc_impl::_reset()
    {
      gr::thread::scoped_lock lock(d_setlock);
      for (int i = 0; i < d_size; i++)
        d_xbuffers[i] = i/d_samp_rate;
      for (int n = 0; n < d_nconnections; n++) {
      	memset(d_buffers[n], 0, d_buffer_size*sizeof(float));
      }
      d_start = 0;
      d_end = d_buffer_size;
      d_index = 0;
    }
  } /* namespace bokehgui */
} /* namespace gr */
