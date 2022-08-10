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
#include <boost/bind/bind.hpp>
#include "freq_sink_f_proc_impl.h"

using namespace boost::placeholders;

namespace gr {
  namespace bokehgui {

    freq_sink_f_proc::sptr
    freq_sink_f_proc::make(int fftsize, int wintype,
                           double fc, double bw,
                           const std::string &name,
                           int nconnections)
    {
      return gnuradio::get_initial_sptr
        (new freq_sink_f_proc_impl(fftsize,
                                   wintype,
                                   fc, bw, name,
                                   nconnections));
    }

    /*
     * The private constructor
     */
    freq_sink_f_proc_impl::freq_sink_f_proc_impl(int fftsize, int wintype,
        double fc, double bw,
        const std::string &name,
        int nconnections)
      : base_sink<float>("freq_sink_f_proc", fftsize, name, nconnections),
      d_wintype((fft::window::win_type)(wintype)),
      d_center_freq(fc), d_bandwidth(bw)
    {
      // Perform fftshift operation;
      // This is usually desired when plotting
      d_shift = true;
      d_fft = new fft::fft_complex_fwd(d_size, true);

      // Used to save FFT values temporarily
      d_fbuf = std::vector<float> (d_size, 0);

      // Used as temporary variable while performing fft shift
      d_tmpbuflen = (unsigned int)(floor(d_size/2.0));
      d_tmpbuf = std::vector<float>(d_tmpbuflen+1, 0);

      message_port_register_in(pmt::mp("freq"));
      set_msg_handler(pmt::mp("freq"),
                      boost::bind(&freq_sink_f_proc_impl::handle_set_freq, this, _1));

      buildwindow();

      set_trigger_mode(TRIG_MODE_FREE, 0, 0, "");
      set_output_multiple(d_size);
    }

    /*
     * Our virtual destructor.
     */
    freq_sink_f_proc_impl::~freq_sink_f_proc_impl()
    {
      delete d_fft;

      d_fbuf = std::vector<float> ();
      d_tmpbuf = std::vector<float> ();
    }

    void
    freq_sink_f_proc_impl::set_trigger_mode(trigger_mode mode,
                                            float level,
                                            int channel,
                                            const std::string &tag_key)
    {
      gr::thread::scoped_lock lock(d_setlock);

      d_trigger_mode = mode;
      d_trigger_level = level;
      d_trigger_channel = channel;
      d_trigger_tag_key = pmt::intern(tag_key);
      d_triggered = false;
      d_trigger_count = 0;

      _reset();
    }

    void
    freq_sink_f_proc_impl::process_plot(float* arr, int* nrows, int* size) {
      if (d_nconnections != 0) { // Not message input. Ignore nconnections+1-th row!
        for(int n = 0; n < *nrows -1; n++) {
          fft(&d_fbuf[0], &d_buffers.front()[n][0], *size);
          for(int x = 0; x < *size; x++) {
            arr[n*(*size) + x] = d_fbuf[x];
          }
        }
      }
      else { // Message input. Calculate FFT using handle_pdu way
        int winoverlap = 4;
        int fftoverlap = d_size / winoverlap;
        float num = static_cast<float>(winoverlap*d_len.front())/static_cast<float>(d_size);
        int nffts = static_cast<int>(ceilf(num));

        size_t min = 0;
        size_t max = std::min(d_size, static_cast<int>(d_len.front()));
        std::vector<float> temp_zero_vec = std::vector<float> (d_size, 0);
        for(int n = 0; n < nffts; n++) {
          // Clear in case (max-min) < d_size
          memset(&temp_zero_vec[0], 0, d_size*sizeof(float));
          //Copy as much possible samples as we can
          memcpy(&temp_zero_vec[0], &d_buffers.front()[0][min], (max-min)*sizeof(float));
          // Apply the window and FFT; copy data into the PDU magnitude buffer
          fft(&d_fbuf[0], &temp_zero_vec[0], d_size);
          for(int x = 0; x < d_size; x++) {
            arr[x] += d_fbuf[x];
          }

          // Increment our indices; set max up to number of samples in the input PDU.
          min += fftoverlap;
          max = std::min(max+fftoverlap, static_cast<size_t>(d_len.front()));
        }
        for(int x = 0; x<d_size; x++) {
          arr[x] /= static_cast<float>(nffts);
        }
        d_len.pop();
      }
    }

    void
    freq_sink_f_proc_impl::set_frequency_range(double centerfreq, double bw) {
      d_center_freq = centerfreq;
      d_bandwidth = bw;
    }

    void
    freq_sink_f_proc_impl::reset()
    {
      gr::thread::scoped_lock lock(d_setlock);
      _reset();
    }

    void
    freq_sink_f_proc_impl::_reset()
    {
      // Reset the trigger
      if(d_trigger_mode == TRIG_MODE_FREE)
        d_triggered = true;
      else
        d_triggered = false;
    }

    void
    freq_sink_f_proc_impl::fft(float *data_out, const float *data_in, int size)
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
      memcpy(&d_tmpbuf[0], &data_out[0], sizeof(float)*(d_tmpbuflen + 1));
      memcpy(&data_out[0], &data_out[size - d_tmpbuflen], sizeof(float)*d_tmpbuflen);
      memcpy(&data_out[d_tmpbuflen], &d_tmpbuf[0], (d_tmpbuflen + 1)*sizeof(float));
    }

    bool
    freq_sink_f_proc_impl::set_fft_window(fft::window::win_type newwintype)
    {
      gr::thread::scoped_lock lock(d_setlock);

      if(d_wintype != newwintype) {
        d_wintype = newwintype;
        buildwindow();
        return true;
      }
      return false;
    }

    void
    freq_sink_f_proc_impl::buildwindow()
    {
      d_window.clear();
      // if(d_wintype != filter::firdes::WIN_NONE) {
        d_window = fft::window::build(d_wintype, d_size, 6.76);
      // }
    }

    void
    freq_sink_f_proc_impl::set_size(const int newsize)
    {
      gr::thread::scoped_lock lock(d_setlock);
      if(newsize != d_size) {
        d_size = newsize;
        d_index = 0;

        // Reset window to reflect new size
        buildwindow();

        // Reset FFTW plan for new size
        delete d_fft;
        d_fft = new fft::fft_complex_fwd(d_size, true);

        d_fbuf = std::vector<float> (d_size, 0);
        d_tmpbuflen = (unsigned int)(floor(d_size/2.0));
        d_tmpbuf = std::vector<float> (d_tmpbuflen, 0);

        set_output_multiple(d_size);

        clear_queue();
      }
    }

    void
    freq_sink_f_proc_impl::handle_set_freq(pmt::pmt_t msg)
    {
      if(pmt::is_pair(msg)) {
        pmt::pmt_t x = pmt::cdr(msg);
        if(pmt::is_real(x)) {
          d_center_freq = pmt::to_double(x);
        }
      }
    }

    void
    freq_sink_f_proc_impl::_test_trigger_tags(int start, int nitems)
    {
      uint64_t nr = nitems_read(d_trigger_channel);
      std::vector<gr::tag_t> tags;
      get_tags_in_range(tags, d_trigger_channel,
                        nr+start, nr+start+nitems,
                        d_trigger_tag_key);
      if(tags.size() > 0) {
        d_triggered = true;
        d_index = tags[0].offset - nr;
        d_trigger_count = 0;
      }
    }

    void
    freq_sink_f_proc_impl::_test_trigger_norm(int nitems, std::vector<std::vector<float> > inputs)
    {
      const float *in = &inputs[d_trigger_channel][0];
      for(int i = 0; i < nitems; i++) {
        d_trigger_count++;

        // Test if trigger has occurred based on the FFT magnitude and
        // channel number. Test if any value is > the level in dBx
        if(in[i] > d_trigger_level) {
          d_triggered = true;
          d_trigger_count = 0;
          break;
        }
      }

      // If using auto trigger mode, trigger peridically even
      // without a trigger event
      if((d_trigger_mode == TRIG_MODE_AUTO) && (d_trigger_count > d_size)) {
        d_triggered = true;
        d_trigger_count = 0;
      }
    }

    void
    freq_sink_f_proc_impl::pop_other_queues() {
    }

    void
    freq_sink_f_proc_impl::verify_datatype_PDU(const float* in, pmt::pmt_t samples, size_t len) {
      if (pmt::is_f32vector(samples)) {
        in = (const float*) pmt::f32vector_elements(samples, len);
      }
      else {
        throw std::runtime_error(d_name + "unknown data type "
                                 "of samples; must be float");
      }
    }

    void
    freq_sink_f_proc_impl::work_process_other_queues(int start, int nitems) {
    }

    double
    freq_sink_f_proc_impl::get_center_freq()
    {
      return d_center_freq;
    }

    double
    freq_sink_f_proc_impl::get_bandwidth()
    {
      return d_bandwidth;
    }

    int
    freq_sink_f_proc_impl::get_wintype()
    {
      return d_wintype;
    }
  } /* namespace bokehgui */
} /* namespace gr */

