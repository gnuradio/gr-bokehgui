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

#include <gnuradio/fft/fft.h>
#include <bokehgui/freq_sink_f_proc.h>

namespace gr {
  namespace bokehgui {

    class freq_sink_f_proc_impl : public freq_sink_f_proc
    {
     private:
       filter::firdes::win_type d_wintype;
       std::vector<float> d_window;
       double d_center_freq, d_bandwidth;
       bool d_shift;
       fft::fft_complex* d_fft;
       std::vector<float> d_fbuf;
       unsigned int d_tmpbuflen;
       std::vector<float> d_tmpbuf;

       // Some freq_sink specific trigger
       float d_trigger_level;
       int d_trigger_count;

     public:
      freq_sink_f_proc_impl(int fftsize,
                            int wintype,
                            double fc, double bw,
                            const std::string &name,
                            int nconnections);
      ~freq_sink_f_proc_impl();

      void set_trigger_mode(trigger_mode mode,
                            float level,
                            int channel,
                            const std::string &tag_key);
      void reset();
      void _reset();
      void fft(float*, const float*, int);
      bool set_fft_window(filter::firdes::win_type newwintype);
      void set_frequency_range(double, double);
      void buildwindow();
      void set_size(int newsize);
      void handle_set_freq(pmt::pmt_t);
      void _test_trigger_tags(int, int);
      void _test_trigger_norm(int, std::vector<std::vector<float> >);

      double get_center_freq();
      double get_bandwidth();
      int get_wintype();

      // Virtual functions inherited from base_sink
      void process_plot(float* arr, int* nrows, int* size);
      void pop_other_queues();
      void verify_datatype_PDU(const float*, pmt::pmt_t, size_t);
      void work_process_other_queues(int start, int nitems);
    };
  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_FREQ_SINK_F_PROC_IMPL_H */

