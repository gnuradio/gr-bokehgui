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

    std::vector<std::vector<gr::tag_t> >
    time_sink_c_proc_impl::get_tags(void) {
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
    time_sink_c_proc_impl::set_trigger_mode(int mode, int slope,
                                       float level,
                                       float delay, int channel,
                                       const std::string &tag_key)
    {
      gr::thread::scoped_lock lock(d_setlock);
      d_trigger_mode = (trigger_mode) mode;
      d_trigger_slope = (trigger_slope) slope;
      d_trigger_level = level;
      d_trigger_delay = static_cast<int>(delay*d_samp_rate);
      d_trigger_channel = channel;
      d_trigger_tag_key = pmt::intern(tag_key);
      d_triggered = false;
      d_trigger_count = 0;

      if((d_trigger_delay < 0) || (d_trigger_delay >= d_size)) {
        GR_LOG_WARN(d_logger, boost::format("Trigger delay (%1%) outside of display range (0:%2%).") \
                    % (d_trigger_delay/d_samp_rate) % ((d_size - 1) / d_samp_rate));
        d_trigger_delay = std::max(0, std::min(d_size - 1, d_trigger_delay));
        delay = d_trigger_delay/d_samp_rate;
      }
      _reset();
    }

    void
    time_sink_c_proc_impl::set_nsamps(const int newsize)
    {
      if(newsize != d_size) {
        gr::thread::scoped_lock lock(d_setlock);
        // Set new size and rest buffer indexes
        // Throws away current data!
        d_size = newsize;
        d_buffer_size = 3*d_size;

        // Resize buffers and replace data
        volk_free(d_xbuffers);
        d_xbuffers = (float*) volk_malloc(d_buffer_size*sizeof(float), volk_get_alignment());
        for (int i = 0; i < d_size; i++)
          d_xbuffers[i] = i/d_samp_rate;

        for (int n = 0; n < d_nconnections + 1; n++) {
          volk_free(d_buffers[2*n]);
          volk_free(d_buffers[2*n+1]);
          d_buffers[2*n] = (float*)volk_malloc(d_buffer_size*sizeof(float),
                                                      volk_get_alignment());
          d_buffers[2*n+1] = (float*)volk_malloc(d_buffer_size*sizeof(float),
                                                      volk_get_alignment());
          memset(d_buffers[2*n], 0, d_buffer_size*sizeof(float));
          memset(d_buffers[2*n+1], 0, d_buffer_size*sizeof(float));
        }

        // If delay was set beyond the new boundary, pull it back.
        if(d_trigger_delay >= d_size) {
          GR_LOG_WARN(d_logger, boost::format("Trigger delay (%1%) outside of display range (0:%2:). Moving to 50%% point.") \
                    % (d_trigger_delay/d_samp_rate) % ((d_size-1)/d_samp_rate));
          d_trigger_delay = d_size - 1;
        }
        _reset();
      }
    }

    void
    time_sink_c_proc_impl::set_samp_rate(const double samp_rate)
    {
      gr::thread::scoped_lock lock(d_setlock);
      d_samp_rate = samp_rate;
      for (int i = 0; i < d_size; i++)
        d_xbuffers[i] = i/d_samp_rate;
    }

    int
    time_sink_c_proc_impl::nsamps() const
    {
      return d_size;
    }

    void
    time_sink_c_proc_impl::reset()
    {
      gr::thread::scoped_lock lock(d_setlock);
      _reset();
    }

    void
    time_sink_c_proc_impl::_reset()
    {
      if(d_trigger_delay) {
        for(int n = 0; n < d_nconnections; n++) {
          // Move the tail of the buffers to the front. This section
          // represents data that might have to be plotted again if a
          // trigger occurs and we have a trigger delay set. The tail
          // section is between (d_end-d_trigger_delay) and d_end.
          memmove(d_buffers[2*n], &d_buffers[n][d_end-d_trigger_delay], d_trigger_delay*sizeof(float));
          memmove(d_buffers[2*n+1], &d_buffers[n][d_end-d_trigger_delay], d_trigger_delay*sizeof(float));
          // Also move the offsets of any tags that occur in the tail
          // section so they would be plotted again, too.
          std::vector<gr::tag_t> tmp_tags;
          for (size_t t = 0; t < d_tags[n].size(); t++) {
            if(d_tags[n][t].offset > (uint64_t)(d_size - d_trigger_delay)) {
              d_tags[n][t].offset = d_tags[n][t].offset - (d_size - d_trigger_delay);
              tmp_tags.push_back(d_tags[n][t]);
            }
          }
          d_tags[n] = tmp_tags;
        }
      }
      // Otherwise clear tags and reset buffer indexes
      else {
        for (int n = 0; n < d_nconnections; n++) {
          d_tags[n].clear();
        }
        d_start = 0;
        d_end = d_buffer_size;
      }

      // Reset the trigger. If in free running mode, ignore the
      // trigger delay and always set trigger to true
      if (d_trigger_mode == TRIG_MODE_FREE) {
        d_index = 0;
        d_triggered = true;
      }
      else {
        d_index = d_trigger_delay;
        d_triggered = false;
      }
    }

    bool
    time_sink_c_proc_impl::_test_trigger_slope(const float *iBuff, const float *qBuff) const
    {
      float x0, x1;
      if(d_trigger_channel % 2 == 0) {
        x0 = iBuff[0];
        x1 = iBuff[1];
      }
      else {
        x0 = qBuff[0];
        x1 = qBuff[1];
      }

      if(d_trigger_slope == TRIG_SLOPE_POS)
        return ((x0 <= d_trigger_level) && (x1 > d_trigger_level));
      else
        return ((x0 >= d_trigger_level) && (x1 < d_trigger_level));
    }

    void
    time_sink_c_proc_impl::_test_trigger_norm()
    {
      int trigger_index;
      const float *iBuff = (const float*) d_buffers[d_trigger_channel/2];
      const float *qBuff = (const float*) d_buffers[d_trigger_channel/2 + 1];
      for(trigger_index = 0; trigger_index < d_size; trigger_index++) {
        d_trigger_count++;

        // Test if trigger has occurred based on the input stream,
        // channel number, and slope direction
        if(_test_trigger_slope(&iBuff[trigger_index],&qBuff[trigger_index])) {
          d_triggered = true;
          d_start = d_index+trigger_index - d_trigger_delay;
          d_end = d_start + d_size;
          d_trigger_count = 0;
          _adjust_tags(-d_start);
          break;
        }
      }

      // If using auto trigger mode, trigger periodically even
      // without a trigger event.
      if((d_trigger_mode == TRIG_MODE_AUTO) && (d_trigger_count > d_size)) {
        d_triggered = true;
        d_trigger_count = 0;
      }
    }

    void
    time_sink_c_proc_impl::_test_trigger_tags()
    {
      int trigger_index;
      std::vector<gr::tag_t> tags;
      // Can't use get_tags_in_range() because it uses input from a port
      // In this case, since the call is from Python, we will check for
      // tags.offset < d_size from thags buffer!
      for(int j = 0; j < d_tags[d_trigger_channel/2].size(); j++) {
        if(d_tags[d_trigger_channel/2][j].offset < d_size) {
          if (pmt::eqv(d_trigger_tag_key, d_tags[d_trigger_channel/2][j].key))
            tags.push_back(d_tags[d_trigger_channel/2][j]);
        }
      }

      if(tags.size() > 0) {
        trigger_index = tags[0].offset;
        int start = trigger_index - d_trigger_delay - 1;
        if (start >= 0) {
          d_triggered = true;
          discard_buffer(start);
          d_trigger_count = 0;
          _adjust_tags(-d_start);
        }
      }
    }

    void
    time_sink_c_proc_impl::discard_buffer(int start) {
      // Function used by tag_trigger_check. Discard initial
      // buffer because initial tags were discarded
      for (int i = 0; i < 2*d_nconnections; i++) {
        memmove(&d_buffers[i], &d_buffers[i][start], (d_index - d_size)*sizeof(float));
      }
    }

    bool
    time_sink_c_proc_impl::is_triggered() {
      // This function will be called by Python just before streaming the data
      // Returns a bool value if trigger is on or not
      // Hence, there will not be the data change! Python will call for the
      // data later. The functions called by this function will check for first d_size data of data_buffer or tags with offset less than d_size in tag_buffer
      // First of all, return false if there is not at least d_size data
      if(d_index < d_size) {
        return false;
      }

      // If auto, normal or tag trigger, look for the trigger
      if((d_trigger_mode != TRIG_MODE_FREE) && !d_triggered) {
        // trigger off a tag key (first one found)
        if (d_trigger_mode == TRIG_MODE_TAG) {
          _test_trigger_tags();
        }
        // Normal or Auto trigger
        else {
          _test_trigger_norm();
        }
      }
      else
        return true;
    }

    void
    time_sink_c_proc_impl::_adjust_tags(int adj)
    {
      for(size_t n = 0; n < d_tags.size(); n++) {
        for(size_t t = 0; t < d_tags[n].size(); t++) {
          d_tags[n][t].offset += adj;
        }
      }
    }

    void
    time_sink_c_proc_impl::handle_pdus(pmt::pmt_t msg) {
      size_t len;
      pmt::pmt_t dict, samples;

      // Test to make sure this is either a PDU or a uniform vector of
      // samples. Get the samples PMT and the dictionary if it's a PDU.
      // If not, we throw an error and exit.
      if(pmt::is_pair(msg)) {
        dict = pmt::car(msg);
        samples = pmt::cdr(msg);
      }
      else if(pmt::is_uniform_vector(msg)) {
        samples = msg;
      }
      else {
        throw std::runtime_error("time_sink_c_proc: message must be either "
                                 "a PDU or a uniform vector of samples.");
      }

      len = pmt::length(samples);
      const gr_complex *in;
      if(pmt::is_c32vector(samples)) {
        in = (const gr_complex*)pmt::c32vector_elements(samples, len);
      }
      else {
        throw std::runtime_error("time_sink_c_proc: unknown data type "
                                 "of samples; must be complex.");
      }

      // Copy data to buffer
      set_nsamps(len);
      volk_32fc_deinterleave_32f_x2(d_buffers[2*d_nconnections],
                                    d_buffers[2*d_nconnections+1],
                                                                      in, len);
      // FIXME:
      // Call to python to plot
    }
  } /* namespace bokehgui */
} /* namespace gr */

