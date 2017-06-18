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
      d_size(size), d_samp_rate(samp_rate), d_name(name),
      d_nconnections(nconnections)
    {
      d_queue_size = 10;
      d_start = 0;
      d_buffers = std::queue<std::pair<gr_complex**, int> > ();

      // setup PDU handling input port
      message_port_register_in(pmt::mp("in"));
      set_msg_handler(pmt::mp("in"),
                      boost::bind(&time_sink_c_proc_impl::handle_pdus, this, _1));

      // Set alignment properties for VOLK
      const int alignment_multiple = volk_get_alignment() / sizeof(gr_complex);
      set_alignment(std::max(1, alignment_multiple));

      set_output_multiple(d_size);

      set_trigger_mode(TRIG_MODE_FREE, TRIG_SLOPE_POS, 0.0, 0.0, 0, "");

      set_history(2);          // so we can look ahead for the trigger slope
      declare_sample_delay(1); // delay the tags for a history of 2
    }

    time_sink_c_proc_impl::~time_sink_c_proc_impl()
    {
      while(!d_buffers.empty())
        d_buffers.pop();
      while(!d_tags.empty())
        d_tags.pop();
    }

    bool
    time_sink_c_proc_impl::check_topology(int ninputs, int noutputs)
    {
      return ninputs == d_nconnections;
    }

    void
    time_sink_c_proc_impl::get_plot_data(gr_complex** output_items, int* nrows, int* size) {
      gr::thread::scoped_lock lock(d_setlock);
      if(!d_buffers.size()) {
        *size = 0;
        *nrows = d_nconnections + 1;
        return;
      }
      *nrows = d_nconnections + 1;
      *size = d_buffers.front().second;

      // 0th row for xbuffer
      // (2*i+1)th row for I-part
      // (2*i+2)th row for Q-part of i-th input; 0 <= i < d_nconnections;
      // (2*d_nconnections + 1)th row for I-part of PDU message
      // (2*d_nconnections + 2)th row for Q-part of PDU message
      gr_complex* arr = (gr_complex*)volk_malloc((*nrows)*(*size)*sizeof(gr_complex), volk_get_alignment());

      for(int n = 0; n < *nrows; n++) {
        memcpy(&arr[n*(*size)], &d_buffers.front().first[n][0], (*size)*sizeof(gr_complex));
      }
      *output_items = arr;
      d_buffers.pop();
      return;
    }

    std::vector<std::vector<gr::tag_t> >
    time_sink_c_proc_impl::get_tags(void) {
      gr::thread::scoped_lock lock(d_setlock);
      if(!d_tags.size()) {
        std::vector<std::vector<gr::tag_t> > tags = std::vector<std::vector<gr::tag_t> > (d_nconnections);
        return tags;
      }
      std::vector<std::vector<gr::tag_t> > tags = d_tags.front();
      d_tags.pop();
      return tags;
    }

    int
    time_sink_c_proc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      gr::thread::scoped_lock lock(d_setlock);
      const gr_complex *in;
      d_start = 0;
      int nitems = std::min(d_size, noutput_items);
      // If auto, normal, or tag trigger, look for the trigger
      if ((d_trigger_mode != TRIG_MODE_FREE) && !d_triggered) {
        // trigger off a tag key (first one found)
        if (d_trigger_mode == TRIG_MODE_TAG) {
          _test_trigger_tags(nitems);
        }
        // Normal or Auto trigger
        else {
          _test_trigger_norm(nitems, input_items);
        }
      }
      // The d_triggered and d_start is set.
      // We will now check if triggered.
      // If it is triggered then we check the trigger index = d_start
      // We will start looking from d_start to the end of input_items
      // First check if d_start+d_size < noutput_items
      if(d_triggered) {
        if (d_start + d_size > noutput_items) {
          // Since, there is already a trigger at d_start,
          // the triggers for next d_size is irrelevant.
          // Hence, we can replace nitems by noutput_items
          nitems = noutput_items - d_start;
        }
        if (d_buffers.size() == d_queue_size) {
          d_buffers.pop();
          d_tags.pop();
        }

        std::pair<gr_complex**, int> pair_buff;
        pair_buff.first = new gr_complex*[d_nconnections+1];
        pair_buff.second = nitems;
        d_buffers.push(pair_buff);

        std::vector<std::vector<gr::tag_t> > tag_buff;
        for(int n = 0; n < d_nconnections + 1; n++) {
          d_buffers.back().first[n] = new gr_complex[nitems];
          memset(d_buffers.back().first[n], 0, nitems*sizeof(gr_complex));
          if (n == d_nconnections) {
            continue;
          }
          else {
            in = (const gr_complex*) input_items[n];
            memcpy(&d_buffers.back().first[n][0], &in[d_start+1], nitems*sizeof(gr_complex));

            uint64_t nr = nitems_read(n);
            std::vector<gr::tag_t> tags;
            get_tags_in_range(tags, n, nr, nr + nitems);
            for(size_t t = 0; t < tags.size(); t++) {
              tags[t].offset = tags[t].offset - nr -d_start-1;
            }
            tag_buff.push_back(tags);
          }
        }
        d_tags.push(tag_buff);
        if(d_trigger_mode != TRIG_MODE_FREE)
          d_triggered = false;
      }
      return nitems + d_start;
    }

    void
    time_sink_c_proc_impl::set_trigger_mode(trigger_mode mode, trigger_slope slope,
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

        // Resize buffers and replace data
        while(!d_buffers.empty()) {
          d_buffers.pop();
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
      // TODO: Different from QT GUI
      // Ignored if d_trigger_delay condition
      // Don't feel it is necessary in current scenario
      while(d_tags.size()) {
        for(int i = 0; i < d_tags.front().size(); i++) {
          d_tags.front()[i].clear();
        }
        d_tags.pop();
      }

      while(!d_buffers.empty()) {
        d_buffers.pop();
      }
      // Reset the trigger. If in free running mode,
      // always set trigger to true
      if (d_trigger_mode == TRIG_MODE_FREE) {
        d_triggered = true;
      }
      else {
        d_triggered = false;
      }
    }

    bool
    time_sink_c_proc_impl::_test_trigger_slope(const gr_complex *in) const
    {
      float x0, x1;
      if(d_trigger_channel % 2 == 0) {
        x0 = in[0].real();
        x1 = in[1].real();
      }
      else {
        x0 = in[0].imag();
        x1 = in[1].imag();
      }

      if(d_trigger_slope == TRIG_SLOPE_POS)
        return ((x0 <= d_trigger_level) && (x1 > d_trigger_level));
      else
        return ((x0 >= d_trigger_level) && (x1 < d_trigger_level));
    }

    void
    time_sink_c_proc_impl::_test_trigger_norm(int nitems, gr_vector_const_void_star inputs)
    {
      int trigger_index;
      const gr_complex *cBuff = (const gr_complex*) inputs[d_trigger_channel];
      for(trigger_index = 0; trigger_index < d_size; trigger_index++) {
        d_trigger_count++;

        // Test if trigger has occurred based on the input stream,
        // channel number, and slope direction
        if(_test_trigger_slope(&cBuff[trigger_index])) {
          d_triggered = true;
          d_start = trigger_index - d_trigger_delay;
          d_trigger_count = 0;
          // _adjust_tags(-d_start);
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
    time_sink_c_proc_impl::_test_trigger_tags(int nitems)
    {
      int trigger_index;
      uint64_t nr = nitems_read(d_trigger_channel);
      std::vector<gr::tag_t> tags;
      get_tags_in_range(tags, d_trigger_channel,
                        nr, nr + nitems + 1,
                        d_trigger_tag_key);
      if(tags.size() > 0) {
        trigger_index = tags[0].offset - nr;
        int start = trigger_index - d_trigger_delay - 1;
        if (start >= 0) {
          d_triggered = true;
          d_start = start;
          d_trigger_count = 0;
//          _adjust_tags(-d_start);
        }
      }
    }

//    void
//    time_sink_c_proc_impl::_adjust_tags(int adj)
//    {
//      for(size_t n = 0; n < d_tags.size(); n++) {
//        for(size_t t = 0; t < d_tags[n].size(); t++) {
//          d_tags[n][t].offset += adj;
//        }
//      }
//    }

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
      if(d_buffers.size() == d_queue_size) {
        d_buffers.pop();
      }

      std::pair<gr_complex**, int> pair_buff;
      pair_buff.first = new gr_complex*[d_nconnections+1];
      pair_buff.second = len;
      d_buffers.push(pair_buff);

      for(int n = 0; n < d_nconnections+1; n++) {
        d_buffers.back().first[n] = new gr_complex[len];
        memset(d_buffers.back().first[n], 0, len*sizeof(gr_complex));
      }
      memcpy(d_buffers.back().first[d_nconnections], &in[0], len*sizeof(gr_complex));
    }
  } /* namespace bokehgui */
} /* namespace gr */

