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
from bokehgui import time_sink_f_proc, utils, bokeh_plot_config

class time_sink_f(bokeh_plot_config):
    """
    docstring for block time_sink_f
    """
    def __init__(self, doc, proc,
                 is_message = False):
        super(time_sink_f, self).__init__()

        self.doc = doc
        self.process = proc

        self.size = self.process.nsamps()
	self.samp_rate = self.process.get_samp_rate()
        self.name = self.process.get_name()
	self.nconnections = self.process.get_nconnections()
        self.is_message = is_message

        self.stream = None
        self.tag_stream = None
        self.plot = None

    def set_trigger_mode(self, trigger_mode, trigger_slope,
                         level, delay, channel, tag_key):
        self.process.set_trigger_mode(trigger_mode,
                                      trigger_slope,
                                      level, delay, channel,
                                      tag_key)

    def initialize(self, log_x = False, log_y = False, legend_list = utils.default_labels_f, update_time = 100):
        y_axis_type = 'log' if log_y else 'linear'
        x_axis_type = 'log' if log_x else 'linear'
        self.plot = figure(tools=utils.default_tools(),
                           active_drag = 'ypan',
                           active_scroll = 'ywheel_zoom',
                           y_axis_type = y_axis_type,
                           x_axis_type = x_axis_type)
        data = dict()
        tag_data = dict()
        data['x'] = []

        if self.is_message:
            nconnection = 1
        else:
            nconnection = self.nconnections

        for i in range(nconnection):
            data['y'+str(i)] = []

            tag_data['x'+str(i)] = []
            tag_data['y'+str(i)] = []
            if not self.is_message:
                tag_data['tags'+str(i)] = []
        self.stream = ColumnDataSource(data)
        self.tag_stream = ColumnDataSource(tag_data)

        self.lines = []
        self.lines_markers = []
        self.legend_list = legend_list[:]
        if not self.is_message:
            self.tags = []
            self.tags_marker = []
        for i in range(nconnection):
            self.lines.append(self.plot.line(
                                        x='x', y='y'+str(i),
                                        source = self.stream,
                                        line_color = 'blue',
                                        legend = self.legend_list[i]
                                        ))
            self.lines_markers.append((None,None))

            if not self.is_message:
                self.tags.append(LabelSet(x='x'+str(i),y='y'+str(i),
                                      text='tags'+str(i),
                                      level='glyph',
                                      x_offset=-20, y_offset=5,
                                      source=self.tag_stream,
                                      text_font_style = 'bold',
                                      text_font_size = '11pt',
                                      render_mode = 'canvas'))
                self.tags_marker.append(self.plot.triangle(
                                                x = 'x'+str(i), y = 'y'+str(i),
                                                source = self.tag_stream,
                                                size = 10, fill_color='red',
                                                ))

                self.plot.add_layout(self.tags[i])

        self.add_custom_tools()
        self.doc.add_root(self.plot)

        if self.name:
            self.set_title(self.name)

        self.doc.add_periodic_callback(self.update, update_time)

    def update(self):
        ## Call to receive from buffers
        output_items = self.process.get_plot_data()
        if not self.is_message:
            tags = self.process.get_tags()
            stream_tags = []
            for i in range(self.nconnections):
                temp_stream_tags = {}
                for j in range(len(tags[i])):
                    temp_stream_tags[tags[i][j].offset] = str(tags[i][j].key) + ":" + str(tags[i][j].value)
                stream_tags.append(temp_stream_tags)

        new_data = dict()
        if self.is_message:
            nconnection = 1
        else:
            nconnection = self.nconnections

        for i in range(nconnection):
            new_data['y'+str(i)] = output_items[i]

        if self.is_message:
            self.size = len(new_data['y0'])
            self.set_x_axis([0, self.size/self.samp_rate])
        new_data['x'] = self.values_x()

        new_tagged_data = {}
        max_tag_size = 0
        for i in range(self.nconnections):
            temp_x = []
            temp_y = []
            temp_tags = []
            for j in stream_tags[i].keys():
                temp_x.append(self.values_x()[j])
                temp_y.append(new_data['y'+str(i)][j])
                temp_tags.append(stream_tags[i][j])
            new_tagged_data['x'+str(i)] = temp_x
            new_tagged_data['y'+str(i)] = temp_y
            new_tagged_data['tags'+str(i)] = temp_tags
            if len(temp_x) > max_tag_size:
                max_tag_size = len(temp_x)

        self.stream.stream(new_data, rollover = self.size)
        self.tag_stream.stream(new_tagged_data, rollover = max_tag_size)
        return

    def values_x(self):
        return [i/float(self.samp_rate) for i in range(self.size)]

    def set_samp_rate(self, samp_rate):
        self.process.set_samp_rate(samp_rate)
        self.samp_rate = samp_rate

    def set_nsamps(self, param, oldsize, newsize):
        if newsize != self.size:
            self.process.set_nsamps(newsize)
            self.size = newsize
        self.set_x_axis([0, self.size/self.samp_rate])

    def enable_tags(self, which = -1, en = True):
        if which == -1:
            for i in range(self.nconnections):
                self.enable_tags(i, en)
        else:
            if en:
                self.tags[which].text_color = 'black'
            else:
                self.tags[which].text_color = None
