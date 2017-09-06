/* -*- c++ -*- */
/*
 * Copyright 2017 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
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


#ifndef INCLUDED_BOKEHGUI_TIME_SINK_F_PROC_H
#define INCLUDED_BOKEHGUI_TIME_SINK_F_PROC_H

#include <bokehgui/base_sink.h>

namespace gr {
  namespace bokehgui {

    /*!
     * \brief A buffer store-sink of multiple signals in time-domain
     * \ingroup bokehgui
     *
     * \details
     * This block is part of Bokeh based time sink for Float values.
     *
     * This is a buffer store that takes set of float streams and save in
     * buffers. The corresponding Python class retrieve the buffer data
     * and plot the signals using Bokeh library.
     *
     * The sink supports storing float data or messages. The message port
     * is named "in". When using message port, \p nconnections should be
     * set to 0. An option for "Float Message" is provided in GRC to use
     * message mode.
     *
     * The sink can plot messages that contain either uniform vectors of
     * float32 values (pmt::is_f32vector) or PDUs where the data is a
     * uniform vector of float32 values.
     *
     */
    class BOKEHGUI_API time_sink_f_proc : virtual public base_sink<float>
    {
    public:
      typedef boost::shared_ptr <time_sink_f_proc> sptr;

      /*!
       * \brief Build floating point time sink
       *
       * \param size number of points to plot at once
       * \param samp_rate sample rate (used to set x-axis labels)
       * \param name title for the plot
       * \param nconnections number of signals connected to sink
       *
       */
      static sptr make(int size, double samp_rate, const std::string &name, int nconnections);

      /*!
       *
       * \brief Called from Python to get the list of tags.
       *
       * The function takes no argument. It returns a 2D list of tags having
       * \p nconnections rows and number of tags in each input port.
       */
      virtual std::vector<std::vector<gr::tag_t> > get_tags() = 0;

      virtual void set_size(const int newsize) = 0;
      virtual void set_samp_rate(const double samp_rate) = 0;
      virtual double get_samp_rate() = 0;

      virtual void reset() = 0;

      /*!
       * Set up a trigger for the sink to know when to save the
       * data. Useful to isolate events and save what is necessary
       *
       * The trigger modes are FREE, AUTO, NORM and TAG
       * (see gr::bokehgui::trigger_mode). The first three are like a
       * normal oscope trigger function. FREE means free running without
       * trigger, AUTO will trigger if the trigger event is seen, but will
       * still plot otherwise, and NORM will hold until the trigger event
       * is observed. The TAG trigger mode allows us to trigger off a specific
       * stream tag. The tag trigger is based on the name of the tag, so
       * when a tag of the given name is seen, the trigger is activated.
       *
       * In auto and normal mode, we look for the slope of the signal.
       * Given a gr::bokehgui::trigger_slope as either Positive or Negative,
       * if the value between two samples moves in the given direction
       * (x[1] > x[0] for Positive or x[1] < x[0] for Negative), then
       * the trigger is activated.
       *
       * The \p delay value is specified in time based off the sample
       * rate. If the sample rate of the block is set to 1, teh delay
       * is then also the sample number offset. This is the offset from
       * the left-hand y-axis of the plot. It delays the signal to show
       * the trigger event at the given delay along with some portion of
       * the signal before the event. The delay must be within 0 - t_max
       * where t_max is the maximum amount of time displayed on the time
       * plot.
       *
       * \param mode trigger_mode: free, auto, normal, tag
       * \param slope trigger_slope: positive or negative. Only
       *              used for auto and normal mode.
       * \param level The magnitude of the trigger even for auto or normal
       *              modes.
       * \param delay The delay (in units of time) for where the trigger happens.
       * \param channel Which input channel to use for the trigger checks
       * \param tag_key The name of the tag to trigger off, if using tag mode
       *
       */
      virtual void set_trigger_mode(trigger_mode mode, trigger_slope slope,
                                    float level,
                                    float delay, int channel,
                                    const std::string &tag_key) = 0;

    };
  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_TIME_SINK_F_PROC_H */
