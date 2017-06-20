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


#ifndef INCLUDED_BOKEHGUI_FREQ_SINK_F_PROC_H
#define INCLUDED_BOKEHGUI_FREQ_SINK_F_PROC_H

#include <bokehgui/api.h>
#include <gnuradio/sync_block.h>
#include <bokehgui/trigger_mode.h>
#include <gnuradio/filter/firdes.h>

namespace gr {
  namespace bokehgui {

    /*!
     * \brief <+description of block+>
     * \ingroup bokehgui
     *
     */
    class BOKEHGUI_API freq_sink_f_proc : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<freq_sink_f_proc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of bokehgui::freq_sink_f_proc.
       *
       * To avoid accidental use of raw pointers, bokehgui::freq_sink_f_proc's
       * constructor is in a private implementation
       * class. bokehgui::freq_sink_f_proc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int fftsize, int wintype, double fc, double bw, const std::string &name, int nconnections);

      virtual void set_trigger_mode(trigger_mode mode,
                            float level,
                            int channel,
                            const std::string &tag_key) = 0;
      virtual void get_plot_data(float** output_items, int* nrows, int* size) = 0;
      virtual void reset() = 0;
      virtual bool set_fft_window(filter::firdes::win_type newwintype) = 0;
      virtual void buildwindow() = 0;
      virtual bool fftresize(const int) = 0;
      virtual void handle_set_freq(pmt::pmt_t) = 0;

      virtual void set_frequency_range(double, double) = 0;
      virtual int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items) = 0;

      virtual void handle_pdus(pmt::pmt_t) = 0;

      virtual double get_center_freq() = 0;
      virtual double get_bandwidth() = 0;
      virtual int get_fft_size() = 0;
      virtual int get_wintype() = 0;
      virtual std::string get_name() = 0;
      virtual int get_nconnections() = 0;
    };

  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_FREQ_SINK_F_PROC_H */

