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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "vec_sink_c_proc_impl.h"

namespace gr {
  namespace bokehgui {

    vec_sink_c_proc::sptr
    vec_sink_c_proc::make(unsigned int vlen,
                           const std::string &name,
                           int nconnections)
    {
      return gnuradio::get_initial_sptr
        (new vec_sink_c_proc_impl(vlen, name, nconnections));
    }

    /*
     * The private constructor
     */
    vec_sink_c_proc_impl::vec_sink_c_proc_impl(unsigned int vlen,
        const std::string &name,
        int nconnections)
      : base_sink<gr_complex>("vec_sink_c_proc", 1, name, nconnections, vlen)
    {

      // message_port_register_in(pmt::mp("freq"));
      // set_msg_handler(pmt::mp("freq"),
      //                 boost::bind(&vec_sink_f_proc_impl::handle_set_freq, this, _1));


      d_trigger_mode = TRIG_MODE_FREE;
      d_triggered = true;
      set_output_multiple(d_size);
    }

    /*
     * Our virtual destructor.
     */
    vec_sink_c_proc_impl::~vec_sink_c_proc_impl()
    {
    }

    void
    vec_sink_c_proc_impl::process_plot(float* arr, int* nrows, int* size) {
      for(int n = 0; n < *nrows; n++) {
        volk_32fc_deinterleave_32f_x2(&arr[(2*n+0)*(*size)],
                                      &arr[(2*n+1)*(*size)],
                                      &d_buffers.front()[n][0], *size);
      }
      *nrows = 2*(*nrows);
    }


    void
    vec_sink_c_proc_impl::reset()
    {
      gr::thread::scoped_lock lock(d_setlock);
      _reset();
    }

    void
    vec_sink_c_proc_impl::_reset()
    {
      // Reset the trigger
      if(d_trigger_mode == TRIG_MODE_FREE)
        d_triggered = true;
      else
        d_triggered = false;
    }

    void
    vec_sink_c_proc_impl::pop_other_queues() {
    }

    void
    vec_sink_c_proc_impl::verify_datatype_PDU(const gr_complex* in, pmt::pmt_t samples, size_t len) {
      if (pmt::is_c32vector(samples)) {
        in = (const gr_complex*) pmt::f32vector_elements(samples, len);
      }
      else {
        throw std::runtime_error(d_name + "unknown data type "
                                 "of samples; must be float");
      }
    }

    void
    vec_sink_c_proc_impl::work_process_other_queues(int start, int nitems) {
    }


    void
    vec_sink_c_proc_impl::set_size(const int newsize)
    {
    }

    void
    vec_sink_c_proc_impl::_test_trigger_tags(int start, int nitems)
    {

    }

  } /* namespace bokehgui */
} /* namespace gr */
