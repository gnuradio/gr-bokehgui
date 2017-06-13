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
from bokehgui import time_sink_f_proc, utils

class time_sink_f():
    """
    docstring for block time_sink_f
    """
    def __init__(self, doc, proc, size,
                 samp_rate, name,
                 nconnections = 1, is_message = False):
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

        for i in range(self.nconnections):
            data['y'+str(i)] = []
            if not self.is_message:
                data['tags'+str(i)] = []
        self.stream = ColumnDataSource(data)

        self.lines = []
        self.lines_markers = []
        self.legend_list = legend_list[:]
        if not self.is_message:
            self.tags = []
        for i in range(self.nconnections):
            self.lines.append(self.plot.line(
                                        x='x', y='y'+str(i),
                                        source = self.stream,
                                        line_color = 'blue',
                                        legend = self.legend_list[i]
                                        ))
            self.lines_markers.append((None,None))

            if not self.is_message:
                self.tags.append(LabelSet(x='x',y='y'+str(i),
                                      text='tags'+str(i),
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
        for i in range(self.nconnections + 1):
            if i == 0:
                new_data['x'] = output_items[i]
                continue
            if self.is_message:
                new_data['y'+str(i-1)] = output_items[i+1]
            else:
                new_data['y'+str(i-1)] = output_items[i]
            if not self.is_message:
                new_data['tags'+str(i-1)] = stream_tags[i-1]
        self.stream.stream(new_data, rollover = self.size)
        return

    def set_samp_rate(self, samp_rate):
        self.process.set_samp_rate(samp_rate);
        self.samp_rate = samp_rate

    def set_nsamps(self, param, oldsize, newsize):
        if newsize != self.size:
            self.process.set_nsamps(newsize);
            self.size = newsize
        self.set_x_axis(0, self.size/self.samp_rate);

    def add_custom_tools(self):
        from bokeh.models import HoverTool
        hover = HoverTool(tooltips = [("x", "$x"),
                                      ("y", "$y")])
        self.plot.add_tools(hover)

    def enable_tags(self, which = -1, en = True):
        if which == -1:
            for i in range(self.nconnections):
                self.enable_tags(i, en)
        else:
            if en:
                self.tags[which].text_color = 'black'
            else:
                self.tags[which].text_color = None


    def set_title(self, name):
        self.plot.title.text = self.name
    def get_title(self):
        return self.plot.title.text
    def set_y_axis(self, lst):
        assert (lst[0]<lst[1])
        self.plot.y_range.start = lst[0]
        self.plot.y_range.end = lst[1]
    def set_x_axis(self, lst):
        assert (lst[0]<lst[1])
        self.plot.x_range.start = lst[0]
        self.plot.x_range.end = lst[1]
    def set_x_label(self, xlabel):
        self.plot.xaxis[0].axis_label = xlabel
    def set_y_label(self, ylabel):
        self.plot.yaxis[0].axis_label = ylabel
    def set_line_label(self, i, label):
        self.legend_list[i] = label
    def get_line_label(self, i):
        return self.legend_list[i]
    def set_line_color(self, i, color):
        self.lines[i].glyph.line_color = color
    def get_line_color(self, i):
        return self.lines[i].glyph.line_color
    def set_line_width(self, i, width):
        self.lines[i].glyph.line_width = width
    def get_line_width(self, i):
        return self.lines[i].glyph.line_width
    def set_line_style(self, i, style):
        if style == 'None':
            self.lines[i].visible = False
            return
        self.lines[i].visible = True
        # solid, dashed, dotted, dotdash, dashdot
        self.lines[i].glyph.line_dash = style
    def get_line_style(self, i):
        if not self.lines[i].glyph.visible:
            return
        # solid, dashed, dotted, dotdash, dashdot
        return self.lines[i].glyph.line_dash
    def set_line_marker(self, i, marker):
        if marker == 'None':
            self.lines_markers[i] = (None, None)
        if marker == '*':
            self.lines_markers[i] = (self.plot.asterisk(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), '*')
        if marker == 'o':
            self.lines_markers[i] = (self.plot.circle(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), 'o')
        if marker == 'o+':
            self.lines_markers[i] = (self.plot.circle_cross(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), 'o+')
        if marker == '+':
            self.lines_markers[i] = (self.plot.cross(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), '+')
        if marker == 'd':
            self.lines_markers[i] = (self.plot.diamond(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), 'd')
        if marker == 'd+':
            self.lines_markers[i] = (self.plot.diamond_cross(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), 'd+')
        if marker == 'v':
            self.lines_markers[i] = (self.plot.inverted_triangle(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), 'v')
        if marker == 's':
            self.lines_markers[i] = (self.plot.square(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), 's')
        if marker == 's+':
            self.lines_markers[i] = (self.plot.square_cross(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), 's+')
        if marker == 'sx':
            self.lines_markers[i] = (self.plot.square_x(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), 'sx')
        if marker == '^':
            self.lines_markers[i] = (self.plot.triangle(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), '^')
        if marker == 'x':
            self.lines_markers[i] = (self.plot.x(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            legend = self.legend_list[i],
                                            ), 'x')
    def get_line_marker(self, i):
        return self.lines_markers[i][1]
    def set_line_alpha(self, i, alpha):
        self.lines[i].glyph.line_alpha = alpha
    def get_line_alpha(self, i):
        return self.lines[i].glyph.line_alpha
    def enable_x_grid(self, en=True):
        if en:
            self.plot.xgrid[0].grid_line_alpha = 1
        else:
            self.plot.xgrid[0].grid_line_alpha = 0
    def enable_y_grid(self, en=True):
        if en:
            self.plot.ygrid.grid_line_alpha = 1
        else:
            self.plot.ygrid.grid_line_alpha = 0
    def enable_grid(self, en = True):
        self.enable_x_grid(en)
        self.enable_y_grid(en)
    def enable_axis_labels(self, en = True):
        if en:
            self.plot.xaxis[0].axis_label = '#000000'
            self.plot.yaxis[0].axis_label_text_color = '#000000'
        else:
            self.plot.xaxis[0].axis_label_text_color = '#FFFFFF'
            self.plot.yaxis[0].axis_label_text_color = '#FFFFFF'
    def disable_legend(self, en = True):
        if en:
            self.plot.legend[0].visible = False
        else:
            self.plot.legend[0].visible = True
    # def set_size(self, height, width);

    def set_update_time(self, miliseconds):
        self.doc.add_periodic_callback(self.update, miliseconds)


