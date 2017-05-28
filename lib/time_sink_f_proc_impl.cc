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

      d_tags = std::vector< std::vector<gr::tag_t> >(d_nconnections);

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
      *size = std::min(d_size, d_index);
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
        memmove(&d_buffers[n][0], &d_buffers[n][*size], (d_end - *size)*sizeof(float));
      }
      d_index -= *size;
      return;
    }

    std::vector<std::vector<gr::tag_t> >
    time_sink_f_proc_impl::get_tags(void) {
      return d_tags;
    }

    int time_sink_f_proc_impl::work (int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items) {
      gr::thread::scoped_lock lock(d_setlock);
      int n=0;
      const float *in;

      int nfill = d_end - d_index; // Room left in buffers
      int nitems = std::min(nfill, noutput_items);
      for (int n=0; n<d_nconnections; n++) {
        in = (const float*) input_items[n];
        memmove(&d_buffers[n][d_index], &in[n], nitems*sizeof(float));
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
        _reset();
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
    time_sink_f_proc_impl::reset()
    {
      gr::thread::scoped_lock lock(d_setlock);
      _reset();
    }

    void
    time_sink_f_proc_impl::_reset()
    {
      for (int i = 0; i < d_size; i++)
        d_xbuffers[i] = i/d_samp_rate;
      for (int n = 0; n < d_nconnections; n++) {
      	memset(d_buffers[n], 0, d_buffer_size*sizeof(float));
      }
      for (int n = 0; n < d_nconnections; n++) {
        d_tags[n].clear();
      }

      d_start = 0;
      d_end = d_buffer_size;
      d_index = 0;
    }

    void
    time_sink_f_proc_impl::_adjust_tags(int adj)
    {
      for(size_t n = 0; n < d_tags.size(); n++) {
        for(size_t t = 0; t < d_tags[n].size(); t++) {
          d_tags[n][t].offset += adj;
        }
      }
    }

  } /* namespace bokehgui */
} /* namespace gr */
