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


#ifndef INCLUDED_BOKEHGUI_FREQ_SINK_F_PROC_H
#define INCLUDED_BOKEHGUI_FREQ_SINK_F_PROC_H

#include <bokehgui/base_sink.h>
#include <gnuradio/fft/window.h>

namespace gr {
  namespace bokehgui {

    /*!
     * \brief A buffer store-sink to store multiple signals in frequency-domain.
     * \ingroup bokehgui
     *
     * \details
     * This block is part of Bokeh based frequency sink for Floating values.
     *
     * This is a buffer store that takes set of a floating point streams
     * and stores the PSD. The corresponding Python class retrieve the
     * buffer data and plot the signals using Bokeh library.
     *
     * A queue of 2D array is maintained. Each 2D array is of size
     * \p nconnection \p x \p d_size. For each call to get the data from
     * Python, first element of queue is sent.
     *
     * The sink supports storing float data or messages. The message port
     * is named "in". When using message port, \p nconnections should be
     * set to 0. An option for "Float Message" is provided in GRC to use
     * message mode.
     *
     * This sink can plot messages that contain either uniform vectors
     * of float 32 values (pmt::is_f32vector) or PDUs where the data
     * is a uniform vector of float 32 values.
     *
     */
    class BOKEHGUI_API freq_sink_f_proc : virtual public base_sink<float>
    {
     public:
      typedef std::shared_ptr<freq_sink_f_proc> sptr;

      /*!
       * \brief Build a floating frequency sink
       *
       * \param fftsize size of the FFT to compute and display. If using
       *        the PDU message port to plot samples, the length of
       *        each PDU must be a multiple of the FFT size.
       * \param wintype type of window to apply (see gr::fft::window::win_type)
       * \param fc center frequency of signal (use for x-axis labels)
       * \param bw bandwidth of signal (used to set x-axis labels)
       * \param name title for the plot
       * \param nconnections number of signals to be connected to the
       *        sink. The PDU message port is always available for a
       *        connection, and this value must be set to 0 if only
       *        the PDU message port is being used.
       */
      static sptr make(int fftsize, int wintype, double fc, double bw, const std::string &name, int nconnections);

      virtual void reset() = 0;

      virtual double get_center_freq() = 0;
      virtual double get_bandwidth() = 0;
      // virtual int get_fft_size() = 0; // Renamed to get_size
      virtual int get_wintype() = 0;
      virtual bool set_fft_window(fft::window::win_type newwintype) = 0;
      virtual void buildwindow() = 0;

      // virtual bool fftresize(const int) = 0; // Renamed to set_size
      virtual void set_frequency_range(double, double) = 0;
      virtual void handle_set_freq(pmt::pmt_t) = 0;

      /*!
       * Set up a trigger for the sink to know when to start
       * plotting. Useful to isolate events and avoid noise.
       *
       * The trigger modes are Free, Auto, Normal, and Tag (see
       * gr::bokehgui::trigger_mode). The first three are like a normal
       * trigger function. Free means free running with no trigger,
       * auto will trigger if the trigger event is seen, but will
       * still plot otherwise, and normal will hold until the trigger
       * event is observed. The Tag trigger mode allows us to trigger
       * off a specific stream tag. The tag trigger is based only on
       * the name of the tag, so when a tag of the given name is seen,
       * the trigger is activated.
       *
       * In auto and normal mode, we look to see if the magnitude of
       * the any FFT point is over the set level.
       *
       * \param mode The trigger_mode: free, auto, normal, or tag.
       * \param level The magnitude of the trigger even for auto or normal modes.
       * \param channel Which input channel to use for the trigger events.
       * \param tag_key The name (as a string) of the tag to trigger off
       *                of if using the tag mode.
       */
      virtual void set_trigger_mode(trigger_mode mode,
                            float level,
                            int channel,
                            const std::string &tag_key) = 0;
    };

  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_FREQ_SINK_F_PROC_H */
