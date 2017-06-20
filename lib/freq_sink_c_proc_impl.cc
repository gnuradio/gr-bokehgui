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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <string.h>
#include <volk/volk.h>
#include "freq_sink_c_proc_impl.h"

namespace gr {
  namespace bokehgui {

    freq_sink_c_proc::sptr
    freq_sink_c_proc::make(int fftsize, int wintype, double fc, double bw, const std::string &name, int nconnections)
    {
      return gnuradio::get_initial_sptr
        (new freq_sink_c_proc_impl(fftsize, wintype, fc, bw, name, nconnections));
    }

    /*
     * The private constructor
     */
    freq_sink_c_proc_impl::freq_sink_c_proc_impl(int fftsize, int wintype, double fc, double bw, const std::string &name, int nconnections)
      : gr::sync_block("freq_sink_c_proc",
              gr::io_signature::make(0, nconnections, sizeof(gr_complex)),
              gr::io_signature::make(0, 0, 0)),
      d_fftsize(fftsize), d_fftavg(1.0),
      d_wintype((filter::firdes::win_type)(wintype)),
      d_center_freq(fc), d_bandwidth(bw), d_name(name),
      d_nconnections(nconnections)
    {
      d_queue_size = 10;

      // TODO: Double click on plot callback

      // Setup PDU handling input port
      message_port_register_in(pmt::mp("in"));
      set_msg_handler(pmt::mp("in"),
                      boost::bind(&freq_sink_c_proc_impl::handle_pdus, this, _1));

      // Perform fftshift operation
      // This is usually desired when plotting
      d_shift = true;
      d_fft = new fft::fft_complex(d_fftsize, true);

      // Used to save FFT values temporarily
      d_fbuf = std::vector<float> (d_fftsize, true);

      // Used as temporary variable while performing fft shift
      d_tmpbuflen = (unsigned int)(floor(d_fftsize/2.0));
      d_tmpbuf = std::vector<float>(d_tmpbuflen+1, 0);

      // Used as temporary storage of input values
      d_residbufs.reserve(d_nconnections + 1);
      for(int n = 0; n < d_nconnections + 1; n++) {
        d_residbufs.push_back(std::vector<gr_complex> (d_fftsize, 0));
      }

      // To check trigger index
      d_index = 0;

      buildwindow();

      set_trigger_mode(TRIG_MODE_FREE, 0, 0, "");

      set_output_multiple(d_fftsize);
    }

    /*
     * Our virtual destructor.
     */
    freq_sink_c_proc_impl::~freq_sink_c_proc_impl()
    {
      delete d_fft;

      d_fbuf = std::vector<float> ();
      d_tmpbuf = std::vector<float> ();

      d_residbufs = std::vector<std::vector<gr_complex> >();

      while(!d_magbufs.empty())
        d_magbufs.pop();
    }

    bool
    freq_sink_c_proc_impl::check_topology(int ninputs, int noutputs)
    {
      return ninputs == d_nconnections;
    }

    void
    freq_sink_c_proc_impl::set_trigger_mode(trigger_mode mode,
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
    freq_sink_c_proc_impl::get_plot_data(float** output_items, int* nrows, int* size) {
      gr::thread::scoped_lock lock(d_setlock);
      if(!d_magbufs.size()) {
        *size = 0;
        *nrows = d_nconnections + 1;
        return;
      }
      *nrows = d_nconnections + 1;
      *size = d_fftsize;

      float* arr = (float*)volk_malloc((*nrows)*(*size)*sizeof(float), volk_get_alignment());
      for(int n = 0; n < *nrows; n++) {
        memcpy(&arr[n*(*size)], &d_magbufs.front()[n][0], (*size)*sizeof(float));
      }
      *output_items = arr;

      d_magbufs.pop();

      return;
    }

    void
    freq_sink_c_proc_impl::set_frequency_range(double centerfreq, double bw) {
      d_center_freq = centerfreq;
      d_bandwidth = bw;
    }

    void
    freq_sink_c_proc_impl::reset()
    {
      gr::thread::scoped_lock lock(d_setlock);
      _reset();
    }

    void
    freq_sink_c_proc_impl::_reset()
    {
      // Reset the trigger
      if(d_trigger_mode == TRIG_MODE_FREE)
        d_triggered = true;
      else
        d_triggered = false;
    }

    void
    freq_sink_c_proc_impl::fft(float *data_out, const gr_complex *data_in, int size)
    {
      if(d_window.size()) {
        volk_32fc_32f_multiply_32fc(d_fft->get_inbuf(), data_in,
                                    &d_window.front(), size);
      }
      else {
      	memcpy(d_fft->get_inbuf(), data_in, sizeof(gr_complex)*size);
      }

      d_fft->execute();     // compute the fft

      volk_32fc_s32f_x2_power_spectral_density_32f(data_out, d_fft->get_outbuf(),
                                                   size, 1.0, size);

      // Perform shift operation
      memcpy(&d_tmpbuf[0], &data_out[0], sizeof(float)*(d_tmpbuflen + 1));
      memcpy(&data_out[0], &data_out[size - d_tmpbuflen], sizeof(float)*(d_tmpbuflen));
      memcpy(&data_out[d_tmpbuflen], &d_tmpbuf[0], sizeof(float)*(d_tmpbuflen + 1));
    }

    bool
    freq_sink_c_proc_impl::set_fft_window(filter::firdes::win_type newwintype)
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
    freq_sink_c_proc_impl::buildwindow()
    {
      d_window.clear();
      if(d_wintype != filter::firdes::WIN_NONE) {
        d_window = filter::firdes::window(d_wintype, d_fftsize, 6.76);
      }
    }

    bool
    freq_sink_c_proc_impl::fftresize(int newsize)
    {
      gr::thread::scoped_lock lock(d_setlock);

      if(newsize != d_fftsize) {
        d_fftsize = newsize;
        d_index = 0;

        // Reset window to reflect new size
        buildwindow();

        // Reset FFTW plan for new size
        delete d_fft;
        d_fft = new fft::fft_complex(d_fftsize, true);

        d_fbuf = std::vector<float> (d_fftsize, 0);
        d_tmpbuflen = (unsigned int) (floor(d_fftsize/2.0));
        d_tmpbuf = std::vector<float> (d_tmpbuflen, 0);

        set_output_multiple(d_fftsize);

        for(int n = 0; n < d_nconnections + 1; n++)
          d_residbufs[n] = std::vector<gr_complex> (d_fftsize, 0);

        while(!d_magbufs.empty())
          d_magbufs.pop();

        return true;
      }
      return false;
    }

    // TODO: Check_clicked

    void
    freq_sink_c_proc_impl::handle_set_freq(pmt::pmt_t msg)
    {
      if(pmt::is_pair(msg)) {
        pmt::pmt_t x = pmt::cdr(msg);
        if(pmt::is_real(x)) {
          d_center_freq = pmt::to_double(x);
        }
      }
    }

    void
    freq_sink_c_proc_impl::_test_trigger_tags(int start, int nitems)
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
    freq_sink_c_proc_impl::_test_trigger_norm(int nitems, std::vector<std::vector<float> > inputs)
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

      // If using auto trigger mode, trigger periodically even
      // without a trigger event
      if((d_trigger_mode == TRIG_MODE_AUTO) && (d_trigger_count > d_fftsize)) {
        d_triggered = true;
        d_trigger_count = 0;
      }
    }

    int
    freq_sink_c_proc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in;
      gr::thread::scoped_lock lock(d_setlock);

      // Consume all possible set of data. Each with size d_fftsize
      for(int d_index = 0; d_index < noutput_items; d_index += d_fftsize) {
        //Trigger off tag, if active
        if((d_trigger_mode == TRIG_MODE_TAG) && !d_triggered) {
          _test_trigger_tags(d_index, d_fftsize);
          if(d_triggered) {
            if((d_index + d_fftsize) >= noutput_items)
              return d_index;
          }
        }
        std::vector<std::vector<float> > data_buff;
        data_buff.reserve(d_nconnections + 1);
        for(int n = 0; n < d_nconnections + 1; n++) {
          data_buff.push_back(std::vector<float> (d_fftsize, 0));

          if (n == d_nconnections)
            continue;

          // Fill up residbuf with d_fftsize number of items
          in = (const gr_complex*) input_items[n];
          memcpy(&d_residbufs[n][0], &in[d_index], d_fftsize*sizeof(gr_complex));
          fft(&d_fbuf[0], &d_residbufs[n][0], d_fftsize);
          for(int x = 0; x < d_fftsize; x++) {
            data_buff[n][x] = (1.0 - d_fftavg)*data_buff[n][x] + (d_fftavg)*d_fbuf[x];
          }
        }

        // Test trigger off signal power in d_magbufs
        if((d_trigger_mode == TRIG_MODE_NORM) || (d_trigger_mode == TRIG_MODE_AUTO)) {
          _test_trigger_norm(d_fftsize, d_magbufs.back());
        }

        // If there is a trigger (FREE always triggers), save array in d_magbufs!
        if(d_triggered) {
          if(d_magbufs.size() == d_queue_size)
            d_magbufs.pop();
          d_magbufs.push(data_buff);
        }
        _reset();
      }
      return noutput_items;
    }

    void
    freq_sink_c_proc_impl::handle_pdus(pmt::pmt_t msg)
    {
      size_t len;
      pmt::pmt_t dict, samples;

      // Test to make sure this is either a PDU or a uniform vector of
      // samples. Get the samples PMT and the dictionary if it's a PDU.
      // If not, we throw an error and exit.
      if(pmt::is_pair(msg)) {
        dict = pmt::car(msg);
        samples = pmt::cdr(msg);
      }
      else if(pmt::is_uniform_vector(msg)) {
        samples = msg;
      }
      else {
        throw std::runtime_error("freq_sink_c: message must be either "
                                 "a PDU or a uniform vetor of samples.");
      }

      len = pmt::length(samples);

      const gr_complex *in;
      if(pmt::is_c32vector(samples)) {
        in = (const gr_complex*)pmt::f32vector_elements(samples, len);
      }
      else {
        throw std::runtime_error("freq_sink_c: unknown data type "
                                 "of samples; must be complex of float.");
      }

      fftresize(len);

      int winoverlap = 4; // Defined in QT freq_sink
      int fftoverlap = d_fftsize / winoverlap;
      float num = static_cast<float>(winoverlap * len) / static_cast<float>(d_fftsize);
      int nffts = static_cast<int>(ceilf(num));

      std::vector<std::vector<float> > data_buff;
      data_buff.reserve(d_nconnections + 1);

      if(d_magbufs.size() == d_queue_size)
        d_magbufs.pop();

      d_magbufs.push(data_buff);

      for (int n = 0; n < d_nconnections + 1; n++) {
        d_magbufs.back().push_back(std::vector<float> (d_fftsize, 0));
      }

      size_t min = 0;
      size_t max = std::min(d_fftsize, static_cast<int>(len));
      for(int n= 0; n < nffts; n++) {
        // Clear in case (max - min) < d_fftsize
        memset(&d_residbufs[d_nconnections][0], 0, d_fftsize*sizeof(gr_complex));

        // Copy in as much of the input samples as we can
        memcpy(&d_residbufs[d_nconnections][0], &in[min], (max-min)*sizeof(gr_complex));

        // Apply the window and FFT; copy data into the PDU
        // magnitude buffer
        fft(&d_fbuf[0], &d_residbufs[d_nconnections][0], d_fftsize);
        for(int x = 0; x < d_fftsize; x++) {
          d_magbufs.back()[d_nconnections][x] += d_fbuf[x];
        }

        // Increment our indices; set max up to the number of
        // samples in the inputPDU.
        min += fftoverlap;
        max = std::min(max + fftoverlap, len);
      }

      // Perform the averaging
      for(int x = 0; x < d_fftsize; x++)
        d_magbufs.back()[d_nconnections][x] /= static_cast<float>(nffts);
    }

    double
    freq_sink_c_proc_impl::get_center_freq()
    {
      return d_center_freq;
    }

    double
    freq_sink_c_proc_impl::get_bandwidth()
    {
      return d_bandwidth;
    }

    int
    freq_sink_c_proc_impl::get_fft_size()
    {
      return d_fftsize;
    }

    int
    freq_sink_c_proc_impl::get_wintype()
    {
      return d_wintype;
    }

    std::string
    freq_sink_c_proc_impl::get_name()
    {
      return d_name;
    }

    int
    freq_sink_c_proc_impl::get_nconnections()
    {
      return d_nconnections;
    }

    void
    freq_sink_c_proc_impl::set_fft_avg(float newavg)
    {
      d_fftavg = newavg;
    }
  } /* namespace bokehgui */
} /* namespace gr */

