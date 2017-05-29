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
      // setup PDU handling input port
      message_port_register_in(pmt::mp("in"));
      set_msg_handler(pmt::mp("in"),
                      boost::bind(&time_sink_f_proc_impl::handle_pdus, this, _1));

      // +1 for the PDU buffer
      for (int n = 0; n < d_nconnections + 1; n++) {
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
      *nrows = d_nconnections + 2;
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
      for (int n=0; n < d_nconnections+1; n++) {
        memmove(&d_buffers[n][0], &d_buffers[n][*size], (d_end - *size)*sizeof(float));
      }
      d_index -= *size;
      return;
    }

    std::vector<std::vector<gr::tag_t> >
    time_sink_f_proc_impl::get_tags(void) {
      gr::thread::scoped_lock lock(d_setlock);

      std::vector<std::vector<gr::tag_t> > tags = std::vector<std::vector<gr::tag_t> > (d_nconnections);

      for(int i = 0; i < d_nconnections; i++) {
        // Not sure if tags are in ascending order
        // Hence, list of indexes of tags sent to plot
        std::vector<int> temp_tag_lst = std::vector<int> ();
        for(int j = 0; j < d_tags[i].size(); j++) {
          if(d_tags[i][j].offset < d_size) {
            tags[i].push_back(d_tags[i][j]);
            temp_tag_lst.push_back(j);
          }
          else {
            d_tags[i][j].offset -= d_size;
          }
        }

        // Delete the sent tags from the d_tags
        for(int j = 0; j < temp_tag_lst.size(); j++) {
          // -j to consider already deleted elements
          // in this loop
          d_tags[i].erase (d_tags[i].begin() + temp_tag_lst[j] - j);
        }
        temp_tag_lst.clear();
      }
      return tags;
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

    void
    time_sink_f_proc_impl::handle_pdus(pmt::pmt_t msg)
    {
      size_t len;
      pmt::pmt_t dict, samples;

      // Test to make sure this is either PDU or Uniform vector of
      // samples. Get the samples PMT and the dictionary if it's a PDU.
      // If not, we throw and error and exit.
      if(pmt::is_pair(msg)) {
        dict = pmt::car(msg);
        samples = pmt::cdr(msg);
      }
      else if(pmt::is_uniform_vector(msg)) {
        samples = msg;
      }
      else {
        throw std::runtime_error("time_sink_f_proc: message must be either "
                                 "a PDU or a uniform vector of samples.");
      }

      len = pmt::length(samples);

      const float *in;
      if(pmt::is_f32vector(samples)) {
        in = (const float*)pmt::f32vector_elements(samples, len);
      }
      else {
        throw std::runtime_error("time_sink_f_proc: unknown data type "
                                 "of samples; must be float.");
      }
      // Copy data to buffer
      set_nsamps(len);
      memcpy(d_buffers[d_nconnections], in, len);
      // FIXME:
      // Call to python to plot
      // Things to note: No Tags in PDU. So, get_plot_data and
      // get_tags will have different rows
    }

  } /* namespace bokehgui */
} /* namespace gr */
