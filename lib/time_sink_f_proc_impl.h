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
//      int d_size, d_queue_size;
      double d_samp_rate;
  //    std::string d_name;
    //  int d_nconnections;

      // 2D array of size (nconn*nitems). The int represents nitems
      //std::queue<std::vector<std::vector<float> > > d_buffers;
      std::queue<std::vector<std::vector<gr::tag_t> > > d_tags;

      // Used during checking a trigger
      // Use output from this index
      //int d_index;

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

      //void get_plot_data (float** output_items, int* nrows, int* size);
      std::vector<std::vector<gr::tag_t> > get_tags();
      // Where all the action really happens
//      int work(int noutput_items,
//         gr_vector_const_void_star &input_items,
//         gr_vector_void_star &output_items);
//      bool check_topology(int ninputs, int noutputs);
      void set_size(const int newsize);
      void set_samp_rate(const double samp_rate);
//      int nsamps() const;
      void reset();
      void _reset();
//      void handle_pdus(pmt::pmt_t);

      void process_plot(float* arr, int nrows, int size);
      void pop_other_queues();
      void verify_datatype_PDU(const float*, pmt::pmt_t, size_t);
      void work_process_other_queues(int start, int nitems);
      void set_trigger_mode(trigger_mode mode, trigger_slope slope,
                            float level,
                            float delay, int channel,
                            const std::string &tag_key);
      bool _test_trigger_slope(const float *input) const;
      void _test_trigger_norm(int, int, gr_vector_const_void_star);
      void _test_trigger_tags(int, int);

      double get_samp_rate();
//      std::string get_name();
//      int get_nconnections();
    };
  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_TIME_SINK_F_PROC_IMPL_H */

