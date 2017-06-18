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
import pmt

from bokeh.plotting import figure
from bokeh.models import ColumnDataSource, LabelSet, Legend

from gnuradio import gr
from bokehgui import time_sink_c_proc, utils, bokeh_plot_config

class time_sink_c(bokeh_plot_config):
    """
    docstring for block time_sink_c
    """
    def __init__(self, doc, proc, size,
                 samp_rate, name,
                 nconnections = 1, is_message = False):
        super(time_sink_c, self).__init__()
        self.doc = doc
        self.size = size
	self.samp_rate = samp_rate
        self.name = name
	self.nconnections = nconnections
        self.stream = None
        self.plot = None
        self.process = proc
        self.is_message = is_message

    def set_trigger_mode(self, trigger_mode, trigger_slope,
                         level, delay, channel, tag_key):
        if trigger_mode == 'FREE':
            trigger_mode_val = 0
        elif trigger_mode == 'AUTO':
            trigger_mode_val = 1
        elif trigger_mode == 'NORM':
            trigger_mode_val = 2
        elif trigger_mode == 'TAG':
            trigger_mode_val = 3
        else:
            raise Exception

        if trigger_slope == 'POS':
            trigger_slope_val = 0
        elif trigger_slope == 'NEG':
            trigger_slope_val = 1
        else:
            raise Exception

        self.process.set_trigger_mode(trigger_mode_val,
                                      trigger_slope_val,
                                      level, delay, channel,
                                      tag_key)

    def initialize(self, log_x = False, log_y = False, legend_list = [], update_time = 100):
        y_axis_type = 'log' if log_y else 'linear'
        x_axis_type = 'log' if log_x else 'linear'
        self.plot = figure(tools=utils.default_tools(),
                           active_drag = 'ypan',
                           active_scroll = 'ywheel_zoom',
                           y_axis_type = y_axis_type,
                           x_axis_type = x_axis_type)
        data = dict()
        data['x'] = []

        for i in range(2*self.nconnections):
            data['y'+str(i)] = []
        if not self.is_message:
            for i in range(self.nconnections):
                data['tags'+str(i)] = []
        self.stream = ColumnDataSource(data)

        self.lines = []
        self.lines_markers = []
        self.legend_list = legend_list[:]
        if not self.is_message:
            self.tags = []
        for i in range(2*self.nconnections):
            self.lines.append(self.plot.line(
                                        x='x', y='y'+str(i),
                                        source = self.stream,
                                        line_color = 'blue',
                                        legend = self.legend_list[i]
                                        ))
            self.lines_markers.append((None,None))

            if not self.is_message:
                self.tags.append(LabelSet(x='x', y='y'+str(i),
                                      text='tags'+str(i/2),
                                      level='glyph',
                                      x_offset=5, y_offset=5,
                                      source=self.stream,
                                      render_mode = 'canvas'))
                self.plot.add_layout(self.tags[i])

        self.add_custom_tools()
        self.doc.add_root(self.plot)

        if self.name:
            self.set_title(self.name)

        self.set_update_time(update_time)

    def update(self):
        ## Call to receive from buffers
        output_items = self.process.get_plot_data()
        if not self.is_message:
            tags = self.process.get_tags()
            stream_tags = []
            for i in range(self.nconnections):
                temp_stream_tags = ["" for k in range(len(output_items[i]))]
                for j in range(len(tags[i])):
                    temp_stream_tags[tags[i][j].offset] = str(tags[i][j].key) + ":" + str(tags[i][j].value)

                stream_tags.append(temp_stream_tags[:])

        new_data = dict()
        for i in range(self.nconnections+1):
            if (not self.is_message) and i == self.nconnections:
                continue
            new_data['y'+str(2*i+0)] = output_items[i].real
            new_data['y'+str(2*i+1)] = output_items[i].imag

            if not self.is_message:
                new_data['tags'+str(i)] = stream_tags[i]
        if self.is_message:
            self.size = len(new_data['y0'])
        new_data['x'] = self.values_x()
        self.stream.stream(new_data, rollover = self.size)
        return

    def values_x(self):
        return [i/float(self.samp_rate) for i in range(self.size)]

    def set_samp_rate(self, samp_rate):
        self.process.set_samp_rate(samp_rate);
        self.samp_rate = samp_rate

    def set_nsamps(self, param, oldsize, newsize):
        if newsize != self.size:
            self.process.set_nsamps(newsize);
            self.size = newsize
        self.set_x_axis(0, self.size/self.samp_rate);

    def enable_tags(self, which = -1, en = True):
        if which == -1:
            for i in range(2*self.nconnections):
                self.enable_tags(i, en)
        else:
            if en:
                self.tags[which].text_color = 'black'
            else:
                self.tags[which].text_color = None

    def set_update_time(self, miliseconds):
        self.update_callback = self.doc.add_periodic_callback(self.update, miliseconds)

