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

#ifndef INCLUDED_BOKEHGUI_VEC_SINK_F_PROC_IMPL_H
#define INCLUDED_BOKEHGUI_VEC_SINK_F_PROC_IMPL_H

#include <gnuradio/bokehgui/vec_sink_f_proc.h>

namespace gr {
  namespace bokehgui {

    class vec_sink_f_proc_impl : public vec_sink_f_proc
    {
     private:

       // Some freq_sink specific trigger
       // float d_trigger_level;
       // int d_trigger_count;

     public:
      vec_sink_f_proc_impl(unsigned int vlen,
                            const std::string &name,
                            int nconnections);
      ~vec_sink_f_proc_impl();

      void reset();
      void _reset();

      // Virtual functions inherited from base_sink
      void process_plot(float* arr, int* nrows, int* size);
      void pop_other_queues();
      void verify_datatype_PDU(const float*, pmt::pmt_t, size_t);
      void work_process_other_queues(int start, int nitems);

      void _test_trigger_tags(int, int);
      void _test_trigger_norm(int, int, gr_vector_const_void_star); // TODO: Support proper triggering here
      void set_size(const int newsize);

    };
  } // namespace bokehgui
} // namespace gr

#endif /* INCLUDED_BOKEHGUI_VEC_SINK_F_PROC_IMPL_H */
