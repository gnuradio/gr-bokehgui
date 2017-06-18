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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "freq_sink_f_proc_impl.h"

namespace gr {
  namespace bokehgui {

    freq_sink_f_proc::sptr
    freq_sink_f_proc::make(int fftsize, int wintype, double fc, double bw, const std::string &name, int nconnections)
    {
      return gnuradio::get_initial_sptr
        (new freq_sink_f_proc_impl(fftsize, wintype, fc, bw, name, nconnections));
    }

    /*
     * The private constructor
     */
    freq_sink_f_proc_impl::freq_sink_f_proc_impl(int fftsize, int wintype, double fc, double bw, const std::string &name, int nconnections)
      : gr::sync_block("freq_sink_f_proc",
              gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
              gr::io_signature::make(0, 0, 0))
    {}

    /*
     * Our virtual destructor.
     */
    freq_sink_f_proc_impl::~freq_sink_f_proc_impl()
    {
    }

    int
    freq_sink_f_proc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];

      // Do <+signal processing+>

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace bokehgui */
} /* namespace gr */

