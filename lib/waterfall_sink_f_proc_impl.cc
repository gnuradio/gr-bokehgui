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
#include "waterfall_sink_f_proc_impl.h"

namespace gr {
  namespace bokehgui {
    waterfall_sink_f_proc::sptr
    waterfall_sink_f_proc::make(int fftsize,
                                int wintype,
                                double fc, double bw,
                                const std::string &name)
    {
      return gnuradio::get_initial_sptr
        (new waterfall_sink_f_proc_impl(fftsize, wintype, fc, bw, name));
    }

    waterfall_sink_f_proc_impl::waterfall_sink_f_proc_impl(int fftsize, int wintype,
   double fc, double bw,
   const std::string &name)
      : base_sink<float>("waterfall_sink_f_proc", fftsize, name, 1),
      d_wintype((fft::window::win_type)(wintype)),
      d_center_freq(fc), d_bandwidth(bw), d_nrows(200)
    {
      d_shift = true;
      d_fft = new fft::fft_complex_fwd(d_size, true);

      // Used to save FFT values
      d_fbuf = std::vector<float> (d_size, 0);

      message_port_register_in(pmt::mp("freq"));
      set_msg_handler(pmt::mp("freq"), [this](pmt::pmt_t msg) { this->handle_set_freq(msg); });

      buildwindow();

      set_output_multiple(d_size);

      d_triggered = true;
      d_trigger_mode = TRIG_MODE_FREE;
    }

    waterfall_sink_f_proc_impl::~waterfall_sink_f_proc_impl()
    {
      delete d_fft;
      d_fbuf = std::vector<float>();
    }

    float *
    waterfall_sink_f_proc_impl::get_plot_data () {
      // Reimplementation of get_plot_data to handle
      // multiple rows to be sent in case of PDU message
      gr::thread::scoped_lock lock(d_setlock);
      int nrows = 0;
      if (!d_buffers.size()) {
        int size = 0;
        nrows = d_nconnections + 1;
        float* arr = (float*) malloc(2*(nrows)*(size)*sizeof(float));
        return arr;
      }
      if (d_nconnections) {
        nrows = d_nconnections + 1;
      }
      else {
        nrows = d_nrows;
      }
      int size = d_buffers.front()[0].size();

      float* arr = (float*) malloc(2*(nrows)*(size)*sizeof(float));
      memset(arr, 0, 2*(nrows)*(size)*sizeof(float));

      process_plot(arr, &nrows, &size);

      d_buffers.pop();

      return arr;
    }

    int waterfall_sink_f_proc_impl::get_buff_size(){
      return d_buffers.front()[0].size();
    }

    int waterfall_sink_f_proc_impl::get_buff_cols(){
      if (d_nconnections) {
        return d_nconnections + 1;
      }
      else {
        return d_nrows;
      }
    }

    void
    waterfall_sink_f_proc_impl::process_plot(float* arr, int* nrows, int* size) {
      if (d_nconnections != 0) {
        // Not message input. Ignore nconnections+1-th row!
        for (int n = 0; n < *nrows - 1; n++) {
          fft(&d_fbuf[0], &d_buffers.front()[n][0], *size);
          for(int x = 0; x < *size; x++) {
            arr[n*(*size) + x] = d_fbuf[x];
          }
        }
      }
      else {
        // Message input
        int stride = std::max(0, (int)(d_len.front() - (*size))/(int)(d_nrows));

        set_time_per_fft(1.0/d_bandwidth * stride);

        int j = 0;
        size_t min = 0;
        size_t max = std::min((*size), static_cast<int>(d_len.front()));
        std::vector<float> temp_zero_vec = std::vector<float> ((*size), 0);
        for(size_t i=0; j < d_nrows; i+=stride) {
          // Clear in case (max -min) < (*size)
          memset(&temp_zero_vec[0], 0, (*size)*sizeof(float));
          // Copy as much as possible samples as we can
          memcpy(&temp_zero_vec[0], &d_buffers.front()[0][min], (max-min)*sizeof(float));
          // Apply the window and FFT; copy data into the PDU magnitude buffer
          fft(&d_fbuf[0], &temp_zero_vec[0], (*size));
          for(int x = 0; x < (*size); x++) {
            arr[j*(*size) + x] += (float)d_fbuf[x];
          }

          // Increment our indices; set max up to number of samples in the input PDU.
          min += stride;
          max = std::min(max + stride, static_cast<size_t>(d_len.front()));
					j++;
        }
        d_len.pop();
      }
    }

    void
    waterfall_sink_f_proc_impl::set_frequency_range(double centerfreq, double bw) {
      d_center_freq = centerfreq;
      d_bandwidth = bw;
    }

    void
    waterfall_sink_f_proc_impl::reset()
    {
      gr::thread::scoped_lock lock(d_setlock);
      _reset();
    }

    void
    waterfall_sink_f_proc_impl::_reset()
    {
      d_triggered = true;
    }

    void
    waterfall_sink_f_proc_impl::fft(float *data_out, const float *data_in, int size)
    {
      // float to complex conversion
      gr_complex *dst = d_fft->get_inbuf();

      std::vector<float> temp_zeros = std::vector<float> (size, 0);

      volk_32f_x2_interleave_32fc(dst, data_in, &temp_zeros[0], size);

      if(d_window.size()) {
        volk_32fc_32f_multiply_32fc(d_fft->get_inbuf(), dst, &d_window.front(), size);
      }

      d_fft->execute(); // Compute the fft
      volk_32fc_s32f_x2_power_spectral_density_32f(data_out, d_fft->get_outbuf(),
                                                   size, 1.0, size);

      // Perform shift operation
      int tmpbuflen = (unsigned int)(floor(d_size/2.0));
      std::vector<float> tmpbuf = std::vector<float> (tmpbuflen + 1, 0);
      memcpy(&tmpbuf[0], &data_out[0], sizeof(float)*(tmpbuflen + 1));
      memcpy(&data_out[0], &data_out[size - tmpbuflen], sizeof(float)*tmpbuflen);
      memcpy(&data_out[tmpbuflen], &tmpbuf[0], (tmpbuflen + 1)*sizeof(float));
    }

    void
    waterfall_sink_f_proc_impl::set_fft_window(fft::window::win_type newwintype)
    {
      gr::thread::scoped_lock lock(d_setlock);
      if (d_wintype != newwintype) {
        d_wintype = newwintype;
        buildwindow();
      }
    }

    void
    waterfall_sink_f_proc_impl::buildwindow()
    {
      // if(d_wintype != fft::window::WIN_NONE) {
        d_window = fft::window::build(d_wintype, d_size, 6.76);
      // }
    }

    void
    waterfall_sink_f_proc_impl::set_size(int newsize)
    {
      gr::thread::scoped_lock lock(d_setlock);
      if(newsize != d_size) {
        d_size = newsize;
        d_index = 0;

        buildwindow();

        delete d_fft;
        d_fft = new fft::fft_complex_fwd(d_size, true);

        d_fbuf = std::vector<float> (d_size, 0);

        set_output_multiple(d_size);
        clear_queue();
      }
    }

    void
    waterfall_sink_f_proc_impl::handle_set_freq(pmt::pmt_t msg)
    {
      if(pmt::is_pair(msg)) {
        pmt::pmt_t x = pmt::cdr(msg);
        if(pmt::is_real(x)) {
          d_center_freq = pmt::to_double(x);
        }
      }
    }

    void
    waterfall_sink_f_proc_impl::_test_trigger_tags(int start, int nitems)
    {
      d_triggered = true;
    }

    void
    waterfall_sink_f_proc_impl::pop_other_queues() {
    }

    void
    waterfall_sink_f_proc_impl::verify_datatype_PDU(const float *in, pmt::pmt_t samples, size_t len) {
      if (pmt::is_f32vector(samples)) {
        in = (const float*) pmt::f32vector_elements(samples, len);
      }
      else {
        throw std::runtime_error(d_name + "unknown data type "
                                 "of samples; must be float");
      }
    }

		void
    waterfall_sink_f_proc_impl::work_process_other_queues(int start, int nitems) {
    }

    double
    waterfall_sink_f_proc_impl::get_center_freq()
    {
      return d_center_freq;
    }

    double
    waterfall_sink_f_proc_impl::get_bandwidth()
    {
      return d_bandwidth;
    }

    gr::fft::window::win_type
    waterfall_sink_f_proc_impl::get_wintype()
    {
      return d_wintype;
    }

    void
    waterfall_sink_f_proc_impl::set_time_per_fft(double val) {
      d_time_per_fft = val;
    }

    double
    waterfall_sink_f_proc_impl::get_time_per_fft() {
      return d_time_per_fft;
    }
  } /* namespace bokehgui */
} /* namespace gr */
