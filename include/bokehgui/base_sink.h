/* -*- c++ -*- */
/*
 * Copyright 2008-2012 Free Software Foundation, Inc.
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

#ifndef INCLUDED_BOKEHGUI_BASE_SINK_PROC_H
#define INCLUDED_BOKEHGUI_BASE_SINK_PROC_H

#include <queue>
#include <bokehgui/api.h>
#include <gnuradio/sync_block.h>
#include <bokehgui/trigger_mode.h>

namespace gr {
  namespace bokehgui {
    template <class T, class U> class base_sink : virtual public sync_block
    {
     public:
      base_sink(std::string class_name, int size, const std::string &name, int nconnections);
//      ~base_sink();

      void get_plot_data (U** output_items, int* nrows, int*size);
      int work(int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);
      void handle_pdus(pmt::pmt_t);

      bool check_topology(int ninputs, int noutputs);
      void clear_queue();
      int get_size();
      std::string get_name();
      int get_nconnections();
      virtual void _test_trigger_tags(int, int) = 0;
      virtual void pop_other_queues() = 0;
      virtual void verify_datatype_PDU(const T*, pmt::pmt_t, size_t) = 0;
      virtual void process_plot(U* arr, int nrows, int size) = 0;
      virtual void work_process_other_queues(int, int) = 0;
      virtual void set_size(int) = 0;
     protected:
      std::queue<std::vector<std::vector<T> > > d_buffers;
      int d_size;
      std::string d_name;
      int d_nconnections;
      int d_index;
      int d_queue_size;
      bool d_triggered;
      trigger_mode d_trigger_mode;
    };
  }
}

#endif /* INCLUDED_BOKEHGUI_BASE_SINK_PROC_H */
