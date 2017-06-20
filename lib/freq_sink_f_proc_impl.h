/* -*- c++ -*- */
/* Copyright 2011-2013,2015 Free Software Foundation, Inc.
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
 */

#ifndef INCLUDED_BOKEHGUI_FREQ_SINK_F_PROC_IMPL_H
#define INCLUDED_BOKEHGUI_FREQ_SINK_F_PROC_IMPL_H

#include <queue>
#include <gnuradio/fft/fft.h>
#include <bokehgui/freq_sink_f_proc.h>

namespace gr {
  namespace bokehgui {

    class freq_sink_f_proc_impl : public freq_sink_f_proc
    {
     private:
       int d_fftsize;
       float d_fftavg;
       filter::firdes::win_type d_wintype;
       std::vector<float> d_window;
       double d_center_freq, d_bandwidth;
       std::string d_name;
       int d_nconnections;
       int d_queue_size;
       bool d_shift;
       fft::fft_complex* d_fft;
       std::vector<float> d_fbuf;
       unsigned int d_tmpbuflen;
       std::vector<float> d_tmpbuf;
       std::vector<std::vector<float> > d_residbufs;
       std::queue<std::vector<std::vector<float> > > d_magbufs;
       int d_index;

       trigger_mode d_trigger_mode;
       float d_trigger_level;
       int d_trigger_channel;
       pmt::pmt_t d_trigger_tag_key;
       bool d_triggered;
       int d_trigger_count;

     public:
      freq_sink_f_proc_impl(int fftsize, int wintype, double fc, double bw, const std::string &name, int nconnections);
      ~freq_sink_f_proc_impl();

      bool check_topology(int, int);
      void set_trigger_mode(trigger_mode mode,
                            float level,
                            int channel,
                            const std::string &tag_key);
      void get_plot_data(float** output_items, int* nrows, int* size);
      void reset();
      void _reset();
      void fft(float*, const float*, int);
      bool set_fft_window(filter::firdes::win_type newwintype);
      void set_frequency_range(double, double);
      void buildwindow();
      bool fftresize(int);
      void handle_set_freq(pmt::pmt_t);
      void _test_trigger_tags(int, int);
      void _test_trigger_norm(int, std::vector<std::vector<float> >);

      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);

      void handle_pdus(pmt::pmt_t);
      
      double get_center_freq();
      double get_bandwidth();
      int get_fft_size();
      int get_wintype();
      std::string get_name();
      int get_nconnections();
      void set_fft_avg(float);
    };

  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_FREQ_SINK_F_PROC_IMPL_H */

