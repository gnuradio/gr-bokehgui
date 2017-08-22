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

#include "time_sink_f_proc_impl.h"

namespace gr {
  namespace bokehgui {

    time_sink_f_proc::sptr
    time_sink_f_proc::make(int size,
                           double samp_rate,
                           const std::string &name,
                           int nconnections)
    {
      return gnuradio::get_initial_sptr
        (new time_sink_f_proc_impl(size, samp_rate,
                                   name, nconnections));
    }

    /*
     * The private constructor
     */
    time_sink_f_proc_impl::time_sink_f_proc_impl(int size,
                                                 double samp_rate,
                                                 const std::string &name,
                                                 int nconnections)
      : base_sink<float>("time_sink_f_proc",
                         size, name, nconnections),
       d_samp_rate(samp_rate)
    {
      set_trigger_mode(TRIG_MODE_FREE, TRIG_SLOPE_POS, 0.0, 0.0, 0, "");

      set_history(2);          // so we can look ahead for the trigger slope
      declare_sample_delay(1); // delay the tags for a history of 2
    }

    /*
     * Our virtual destructor.
     */
    time_sink_f_proc_impl::~time_sink_f_proc_impl()
    {
      while(!d_tags.empty())
        d_tags.pop();
    }

    void
    time_sink_f_proc_impl::process_plot(float* arr, int* nrows, int* size) {
      for(int n = 0; n < *nrows; n++) {
        memcpy(&arr[n*(*size)], &d_buffers.front()[n][0], (*size)*sizeof(float));
      }
    }

    void
    time_sink_f_proc_impl::pop_other_queues() {
      d_tags.pop();
    }

    void
    time_sink_f_proc_impl::verify_datatype_PDU(const float* in, pmt::pmt_t samples, size_t len) {
      if(pmt::is_f32vector(samples)) {
        in = (const float*)pmt::f32vector_elements(samples, len);
      }
      else {
        throw std::runtime_error(d_name + "unknown data type "
                                 "of samples; must be float");
      }
      set_size(len);
    }

    std::vector<std::vector<gr::tag_t> >
    time_sink_f_proc_impl::get_tags(void) {
      gr::thread::scoped_lock lock(d_setlock);
      if(!d_tags.size()) {
        std::vector<std::vector<gr::tag_t> > tags = std::vector<std::vector<gr::tag_t> >(d_nconnections);
        return tags;
      }

      std::vector<std::vector<gr::tag_t> > tags = d_tags.front();

      d_tags.pop();

      return tags;
    }

    void
    time_sink_f_proc_impl::work_process_other_queues(int start, int nitems) {
      std::vector<std::vector<gr::tag_t> > tag_buff;
      tag_buff.reserve(d_nconnections);

      d_tags.push(tag_buff);
      for(int n = 0; n < d_nconnections; n++) {
        d_tags.back().push_back(std::vector<gr::tag_t> ());
        uint64_t nr = nitems_read(n);
        get_tags_in_range(d_tags.back()[n], n, nr + d_index, nr + d_index + nitems);
        for(size_t t = 0; t < d_tags.back()[n].size(); t++) {
          d_tags.back()[n][t].offset = d_tags.back()[n][t].offset - nr - d_index - 1;
        }
      }
    }

    void
    time_sink_f_proc_impl::set_trigger_mode(trigger_mode mode, trigger_slope slope,
                                            float level,
                                            float delay, int channel,
                                            const std::string &tag_key)
    {
      gr::thread::scoped_lock lock(d_setlock);

      d_trigger_mode =  mode;
      d_trigger_slope = slope;
      d_trigger_level = level;
      d_trigger_delay = static_cast<int>(delay*d_samp_rate);
      d_trigger_channel = channel;
      d_trigger_tag_key = pmt::intern(tag_key);
      d_triggered = false;
      d_trigger_count = 0;

      if((d_trigger_delay < 0) || (d_trigger_delay >= d_size)) {
        GR_LOG_WARN(d_logger, boost::format("Trigger delay (%1%) outside of display range (0:%2%).") \
                    % (d_trigger_delay/d_samp_rate) % ((d_size-1)/d_samp_rate));
        d_trigger_delay = std::max(0, std::min(d_size-1, d_trigger_delay));
        delay = d_trigger_delay/d_samp_rate;
      }

      _reset();
    }

    void
    time_sink_f_proc_impl::set_size(const int newsize)
    {
      if(newsize != d_size) {
        gr::thread::scoped_lock lock(d_setlock);
        // Set new size and rest buffer indexs.
        // Throws away current data!
        d_size = newsize;
        set_output_multiple(d_size);

        // Resize buffers and relapce data
        clear_queue();

        // If delay was set beyond the new boundary, pull it back.
        if(d_trigger_delay >= d_size) {
          GR_LOG_WARN(d_logger, boost::format("Trigger delay (%1%) outside of display range (0:%2%). Moving to 50%% point.") \
                      % (d_trigger_delay/d_samp_rate) % ((d_size-1)/d_samp_rate));
          d_trigger_delay = d_size-1;
        }
        _reset();
      }
    }

    void
    time_sink_f_proc_impl::set_samp_rate(const double samp_rate)
    {
      gr::thread::scoped_lock lock(d_setlock);
      d_samp_rate = samp_rate;
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
      while(!d_tags.empty())
        d_tags.pop();

      clear_queue();

      // Reset the trigger. If in free running mode,
      // always set trigger to true
      if(d_trigger_mode == TRIG_MODE_FREE) {
        d_triggered = true;
      }
      else {
        d_triggered = false;
      }
    }

    bool
    time_sink_f_proc_impl::_test_trigger_slope(const float *input) const
    {
      float x0, x1;
      x0 = input[0];
      x1 = input[1];

      if(d_trigger_slope == TRIG_SLOPE_POS)
        return ((x0 <= d_trigger_level) && (x1 > d_trigger_level));
      else
        return ((x0 >= d_trigger_level) && (x1 < d_trigger_level));
    }

    void
    time_sink_f_proc_impl::_test_trigger_norm(int start, int nitems, gr_vector_const_void_star inputs)
    {
      int trigger_index;
      const float *in = (const float*)inputs[d_trigger_channel];
      for(trigger_index = start;
          trigger_index < nitems + start;
          trigger_index++) {
        d_trigger_count++;

        // Test if trigger has occurred based on the input stream,
        // channel number, and slope direction
        if(_test_trigger_slope(&in[trigger_index])) {
          d_triggered = true;
          d_index = trigger_index;
          d_trigger_count = 0;
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
    time_sink_f_proc_impl::_test_trigger_tags(int start, int nitems)
    {
      int trigger_index;
      uint64_t nr = nitems_read(d_trigger_channel);
      std::vector<gr::tag_t> tags;
      get_tags_in_range(tags, d_trigger_channel,
                        nr+start, nr + start + nitems,
                        d_trigger_tag_key);
      if(tags.size() > 0) {
        trigger_index = tags[0].offset - nr;
        int start_point = trigger_index - d_trigger_delay - 1;
        if (start_point >= 0) {
            d_triggered = true;
            d_index = trigger_index;
            d_trigger_count = 0;
        }
      }
    }

    double
    time_sink_f_proc_impl::get_samp_rate()
    {
      return d_samp_rate;
    }

  } /* namespace bokehgui */
} /* namespace gr */
