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

#ifndef INCLUDED_BOKEHGUI_TIME_SINK_F_PROC_IMPL_H
#define INCLUDED_BOKEHGUI_TIME_SINK_F_PROC_IMPL_H

#include <bokehgui/time_sink_f_proc.h>

namespace gr {
  namespace bokehgui {

    class time_sink_f_proc_impl : public time_sink_f_proc
    {
     private:
      int d_size, d_buffer_size;
      double d_samp_rate;
      std::string d_name;
      int d_nconnections;

      int d_index, d_start, d_end;
      std::vector<float*> d_buffers;
      float* d_xbuffers;
      std::vector< std::vector<gr::tag_t> > d_tags;

      // Members used for triggering scope
      trigger_mode d_trigger_mode;
      trigger_slope d_trigger_slope;
      float d_trigger_level;
      int d_trigger_channel;
      int d_trigger_delay;
      pmt::pmt_t d_trigger_tag_key;
      bool d_triggered;
      int d_trigger_count;
      int d_initial_delay; // used for limiting d_trigger_delay

     public:
      time_sink_f_proc_impl(int size, double samp_rate, const std::string &name, int nconnections);
      ~time_sink_f_proc_impl();

      void get_plot_data (float** output_items, int* nrows, int* size);
      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
      bool check_topology(int ninputs, int noutputs);
      void set_nsamps(const int newsize);
      void set_samp_rate(const double samp_rate);
      int nsamps() const;
      void reset();
      void _reset();
      void _adjust_tags(int adj);
      std::vector<std::vector<gr::tag_t> > get_tags();
      void handle_pdus(pmt::pmt_t);

      void set_trigger_mode(int mode, int slope,
                            float level,
                            float delay, int channel,
                            const std::string &tag_key);
      bool _test_trigger_slope(const float *input) const;
      void _test_trigger_norm();
      void _test_trigger_tags();
      void discard_buffer(int start);
      bool is_triggered ();
    };
  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_TIME_SINK_F_PROC_IMPL_H */

