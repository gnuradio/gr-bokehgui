# Copyright 2008-2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

import numpy

from bokeh.plotting import figure
from bokeh.models import ColumnDataSource, LabelSet

from gnuradio import gr
from bokehgui import freq_sink_f_proc, utils, bokeh_plot_config

class freq_sink_f(bokeh_plot_config):
    """
    docstring for block freq_sink_f
    """
    def __init__(self, doc, proc,
                 is_message = False):
        super(freq_sink_f, self).__init__()

        self.doc = doc
        self.process = proc

        self.fftsize = self.process.get_fft_size()
        self.wintype = self.process.get_wintype()
        self.name = self.process.get_name()
        self.nconnections = self.process.get_nconnections()
        self.fc = self.process.get_center_freq()
        self.bw = self.process.get_bandwidth()

        self.is_message = is_message

        self.stream = None
        self.plot = None

        self.set_frequency_range(self.fc, self.bw, set_x_axis = False, notify_process = False)

    def set_trigger_mode(self, trigger_mode, level, channel, tag_key):
        self.process.set_trigger_mode(trigger_mode, level,
                                      channel, tag_key)

    def initialize(self, legend_list = [], update_time = 100):
        self.plot = figure(tools = utils.default_tools(),
                            active_drag = 'ypan',
                            active_scroll = 'ywheel_zoom',)
        data = dict()
        data['x'] = []

        for i in range(self.nconnections):
            data['y'+str(i)] = []

        self.stream = ColumnDataSource(data)

        self.lines = []
        self.lines_markers = []
        self.legend_list = legend_list[:]
        for i in range(self.nconnections):
            self.lines.append(self.plot.line(
                                        x = 'x', y='y'+str(i),
                                        source = self.stream,
                                        line_color = 'blue',
                                        legend = self.legend_list[i]
                                        ))
            self.lines_markers.append((None, None))

        self.add_custom_tools()
        self.doc.add_root(self.plot)

        if self.name:
            self.set_title(self.name)

        self.doc.add_periodic_callback(self.update, update_time)

    def update(self):
        ## Call to receive from buffers

        ## First call to check if BW and FC is not changed
        ## Through some direct message port to the block
        output_items = self.process.get_plot_data()
        if len(output_items[0]) != 0:
            fc = self.process.get_center_freq()
            bw = self.process.get_bandwidth()
            fftsize = self.process.get_fft_size()
            if self.fc != fc or self.bw != bw or self.fftsize != fftsize:
                self.fftsize = fftsize
                self.set_frequency_range(fc, bw, notify_process = False)


            new_data = dict()
            for i in range(self.nconnections + 1):
                if(not self.is_message) and i == self.nconnections:
                    continue
                new_data['y'+str(i)] = output_items[i]
            new_data['x'] = self.frequency_range
            self.stream.stream(new_data, rollover = self.fftsize)
        return

    def set_frequency_range(self, fc, bw, set_x_axis = True, notify_process = True):
        self.fc = fc
        self.bw = bw
        if notify_process:
            self.process.set_frequency_range(fc, bw)

        step = bw/self.fftsize

        self.frequency_range = [0]*self.fftsize
        for i in range(self.fftsize/2):
            self.frequency_range[i] = fc - step*(self.fftsize/2 - i)
        self.frequency_range[(self.fftsize + 1)/2] = self.fc
        for i in range((self.fftsize-1)/2):
            self.frequency_range[i + 1 + (self.fftsize+1)/2] = fc + step*i

        if set_x_axis:
            self.set_x_axis([fc - bw/2, fc + bw/2])

    def set_center_freq(self, fc):
        if self.fc != fc:
            self.fc = fc
            self.set_frequency_range(fc, self.bw)

    def set_bandwidth(self, bw):
        if self.bw != bw:
            self.bw = bw
            self.set_frequency_range(self.fc, bw)

    def set_fft_window(self, wintype):
        if not self.wintype == wintype:
            self.wintype = wintype
            self.process.set_fft_window(wintype)

    def set_fft_size(self, fftsize):
        if fftsize < 16 or fftsize > 16384:
            raise ValueError("FreqSink: FFT Size must be between 16 to 16384")
        self.fftsize = fftsize
        self.process.fftresize(fftsize)

    def set_fft_avg(self, newavg):
        self.process.set_fft_avg(newavg)
