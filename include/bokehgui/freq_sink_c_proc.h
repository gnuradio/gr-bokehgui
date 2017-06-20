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


#ifndef INCLUDED_BOKEHGUI_FREQ_SINK_C_PROC_H
#define INCLUDED_BOKEHGUI_FREQ_SINK_C_PROC_H

#include <bokehgui/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace bokehgui {

    /*!
     * \brief <+description of block+>
     * \ingroup bokehgui
     *
     */
    class BOKEHGUI_API freq_sink_c_proc : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<freq_sink_c_proc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of bokehgui::freq_sink_c_proc.
       *
       * To avoid accidental use of raw pointers, bokehgui::freq_sink_c_proc's
       * constructor is in a private implementation
       * class. bokehgui::freq_sink_c_proc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int fftsize, int wintype, double fc, double bw, const std::string &name, int nconnections);
    };

  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_FREQ_SINK_C_PROC_H */

