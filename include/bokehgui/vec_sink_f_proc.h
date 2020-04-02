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


#ifndef INCLUDED_BOKEHGUI_VEC_SINK_F_PROC_H
#define INCLUDED_BOKEHGUI_VEC_SINK_F_PROC_H

#include <bokehgui/base_sink.h>

namespace gr {
  namespace bokehgui {

    /*!
     * \brief A buffer store-sink to store multiple signals as vectors.
     * \ingroup bokehgui
     *
     * \details
     * This block is part of Bokeh based vector sink for Floating values.
     *
     * This is a buffer store that takes set of a floating point vector streams.
     * The corresponding Python class retrieve the
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
    class BOKEHGUI_API vec_sink_f_proc : virtual public base_sink<float>
    {
     public:
      typedef boost::shared_ptr<vec_sink_f_proc> sptr;

      /*!
       * \brief Build a floating point vector sink
       *
       * \param vlen size of the input vectors. If using
       *        the PDU message port to plot samples, the length of
       *        each PDU must be a multiple of the vlen.
       * \param name title for the plot
       * \param nconnections number of signals to be connected to the
       *        sink. The PDU message port is always available for a
       *        connection, and this value must be set to 0 if only
       *        the PDU message port is being used.
       */
      static sptr make(unsigned int vlen, const std::string &name, int nconnections);

      virtual void reset() = 0;

    };

  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_VEC_SINK_F_PROC_H */
