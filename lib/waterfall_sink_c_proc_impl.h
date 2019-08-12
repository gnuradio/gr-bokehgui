/* -*- c++ -*- */
/* Copyright 2017 Free Software Foundation, Inc.
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

#ifndef INCLUDED_BOKEHGUI_WATERFALL_SINK_C_PROC_IMPL_H
#define INCLUDED_BOKEHGUI_WATERFALL_SINK_C_PROC_IMPL_H

#include <bokehgui/waterfall_sink_c_proc.h>
#include <gnuradio/fft/fft.h>

namespace gr {
  namespace bokehgui {
    class BOKEHGUI_API waterfall_sink_c_proc_impl : public waterfall_sink_c_proc
    {
     private:
      int d_nrows;
      float d_fftavg;
      fft::window::win_type d_wintype;
      std::vector<float> d_window;
      double d_center_freq, d_bandwidth;
      bool d_shift;
      fft::fft_complex* d_fft;
      std::vector<float> d_fbuf;
      double d_time_per_fft;

     public:
      waterfall_sink_c_proc_impl(int fftsize, int wintype, double fc, double bw, const std::string &name);
      ~waterfall_sink_c_proc_impl();

      void reset();
      void _reset();
      void fft(float *data_out, const gr_complex *data_in, int size);

      // Handles messages input port
      void handle_set_freq(pmt::pmt_t);

      void set_fft_window(gr::fft::window::win_type win);
      gr::fft::window::win_type get_wintype();

      void set_frequency_range(double, double);
      void _test_trigger_tags(int, int);
      double get_center_freq();
      double get_bandwidth();
      double get_time_per_fft();
      void set_time_per_fft(double);
      void set_fft_avg(float);
      void set_size(int);
      void buildwindow();

      // Virtual functions inherited from base_sink
      void get_plot_data (float** output_items, int* nrows, int* size);
      void process_plot(float* arr, int* nrows, int* size);
      void pop_other_queues();
      void verify_datatype_PDU(const gr_complex*, pmt::pmt_t, size_t);
      void work_process_other_queues(int start, int nitems);
    };
  }
}

#endif
