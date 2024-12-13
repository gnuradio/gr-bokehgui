/* -*- c++ -*- */
/*
 * Copyright 2017 Free Software Foundation, Inc.
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
 */

#ifndef INCLUDED_BOKEHGUI_WATERFALL_SINK_F_H
#define INCLUDED_BOKEHGUI_WATERFALL_SINK_F_H

#include <gnuradio/bokehgui/base_sink.h>
#include <gnuradio/fft/window.h>

namespace gr {
  namespace bokehgui {
    class BOKEHGUI_API waterfall_sink_f_proc : virtual public base_sink<float>
    {
    public:
      typedef std::shared_ptr <waterfall_sink_f_proc> sptr;
      static sptr make(int size,
                       int wintype,
                       double fc, double bw,
                       const std::string &name);

      virtual void reset() = 0;

      virtual void set_size(int) = 0;
      virtual double get_center_freq() = 0;
      virtual double get_bandwidth() = 0;
      virtual double get_time_per_fft() = 0;
      virtual void buildwindow() = 0;
      virtual void set_time_per_fft(double) = 0;
      virtual void set_fft_window(const gr::fft::window::win_type win) = 0;
      virtual gr::fft::window::win_type get_wintype() = 0;

      virtual void set_frequency_range(double, double) = 0;
      virtual float * get_plot_data () = 0;
    };
  }
}

#endif /* INCLUDED_BOKEHGUI_WATERFALL_SINK_F_H */
