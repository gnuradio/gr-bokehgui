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

#ifndef INCLUDED_BOKEHGUI_FREQ_SINK_C_PROC_IMPL_H
#define INCLUDED_BOKEHGUI_FREQ_SINK_C_PROC_IMPL_H

#include <bokehgui/freq_sink_c_proc.h>

namespace gr {
  namespace bokehgui {

    class freq_sink_c_proc_impl : public freq_sink_c_proc
    {
     private:
      // Nothing to declare in this block.

     public:
      freq_sink_c_proc_impl(int fftsize, int wintype, double fc, double bw, const std::string &name, int nconnections);
      ~freq_sink_c_proc_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_FREQ_SINK_C_PROC_IMPL_H */

