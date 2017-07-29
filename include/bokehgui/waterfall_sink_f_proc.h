/* -*- c++ -*- */
/*
 * Copyright 2012,2014-2015 Free Software Foundation, Inc.
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

#include "base_sink.h"
#include <gnuradio/filter/firdes.h>

namespace gr {
  namespace bokehgui {
    class BOKEHGUI_API waterfall_sink_f_proc : virtual public base_sink<float, float>
    {
    public:
      typedef boost::shared_ptr <waterfall_sink_f_proc> sptr;
      static sptr make(int size, int wintype, double fc, double bw, const std::string &name, int nconnections);

      virtual void reset() = 0;

      virtual void set_size(int) = 0;
      virtual void set_time_per_fft(const double t) = 0;
      virtual void set_fft_avg(const float fftavg) = 0;
      virtual void set_fft_window(const gr::filter::firdes::win_type win) = 0;
      virtual gr::filter::firdes::win_type fft_window() = 0;

      virtual void set_frequency_range(double, double) = 0;
    };
  }
}
#endif
