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
from bokeh.models import ColumnDataSource

from gnuradio import gr
from bokehgui import time_sink_f_proc, utils

class time_sink_f():
    """
    docstring for block time_sink_f
    """
    def __init__(self, doc, proc, size,
                 samp_rate, name,
                 nconnections = 1):
        self.doc = doc
        self.size = size
	self.samp_rate = samp_rate
        self.name = name
	self.nconnections = nconnections
        self.stream = None
        self.plot = None
        self.process = proc

        self.initialize()

    def initialize(self):
        self.plot = figure(tools=utils.default_tools(),
                           active_drag = 'ypan',
                           active_scroll = 'ywheel_zoom')
        data = dict()
        data['x'] = []
        for i in range(self.nconnections):
            data['y'+str(i)] = []
        self.stream = ColumnDataSource(data)

        self.lines = []
        self.lines_markers = []
        for i in range(self.nconnections):
            self.lines.append(self.plot.line(
                                        x='x', y='y'+str(i),
                                        source = self.stream,
                                        line_color = 'red' if i==0 else 'blue'
                                        ))
            self.lines_markers.append((None,None))
        self.add_custom_tools()
        self.doc.add_root(self.plot)

        if self.name:
            self.set_title(self.name)

        self.doc.add_periodic_callback(self.update, 200)

    def update(self):
        ## Call to receive from buffers!
        output_items = self.process.get_plot_data()
        new_data = dict()
        for i in range(len(output_items)-1):
            if i == 0:
                new_data['x'] = output_items[i]
                continue
            new_data['y'+str(i-1)] = output_items[i]
        self.stream.stream(new_data, rollover = self.size)

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

    def enable_tags(self, which, en):
        if which == -1:
            for i in range(self.nconnections):
                enable_tags(n, en)
        else:
            pass


    def set_title(self, name):
        self.plot.title.text = self.name
    def set_y_axis(self, lst):
        assert (lst[0]<lst[1])
        self.plot.y_range.start = lst[0]
        self.plot.y_range.end = lst[1]
    def set_x_axis(self, lst):
        assert (lst[0]<lst[1])
        self.plot.x_range.start = lst[0]
        self.plot.x_range.end = lst[1]
    def set_x_label(self, xlabel):
        self.plot.set(xlabel = xlabel)
    def set_y_label(self, ylabel):
        self.plot.set(ylabel = ylabel)
    def set_line_label(self, i, label):
        self.lines[i].legend = label
    def get_line_label(self, i):
        return self.lines[i].legend
    def set_line_color(self, i, color):
        self.lines[i].line_color = color
    def get_line_color(self, i):
        return self.lines[i].line_color
    def set_line_width(self, i, width):
        self.lines[i].line_width = width
    def get_line_width(self, i):
        return self.lines[i].line_width
    def set_line_style(self, i, style):
        # solid, dashed, dotted, dotdash, dashdot
        self.lines[i].line_dash = style
    def get_line_style(self, i):
        # solid, dashed, dotted, dotdash, dashdot
        return self.lines[i].line_dash
    def set_line_marker(self, i, marker):
        if marker == '*':
            self.lines_markers[i] = (self.plot.asterisk(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), '*')
        if marker == 'o':
            self.lines_markers[i] = (self.plot.circle(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), 'o')
        if marker == 'o+':
            self.lines_markers[i] = (self.plot.circle_cross(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), 'o+')
        if marker == '+':
            self.lines_markers[i] = (self.plot.cross(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), '+')
        if marker == 'd':
            self.lines_markers[i] = (self.plot.diamond(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), 'd')
        if marker == 'd+':
            self.lines_markers[i] = (self.plot.diamond_cross(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), 'd+')
        if marker == 'v':
            self.lines_markers[i] = (self.plot.inverted_triangle(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), 'v')
        if marker == 's':
            self.lines_markers[i] = (self.plot.square(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), 's')
        if marker == 's+':
            self.lines_markers[i] = (self.plot.square_cross(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), 's+')
        if marker == 'sx':
            self.lines_markers[i] = (self.plot.square_x(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), 'sx')
        if marker == '^':
            self.lines_markers[i] = (self.plot.triangle(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), '^')
        if marker == 'x':
            self.lines_markers[i] = (self.plot.x(
                                            x='x', y='y'+str(i),
                                            source=self.stream,
                                            hover_fill_color="firebrick",
                                            hover_alpha=0.3), 'x')
    def get_line_marker(self, i):
        return self.lines_markers[i][1]
    def set_line_alpha(self, i, alpha):
        self.lines[i].line_alpha = alpha
    def get_line_alpha(self, i):
        return self.lines[i].line_alpha
    def enable_x_grid(en=True):
        if en:
            self.plot.xgrid.grid_line_width = 1
        else:
            self.plot.xgrid.grid_line_width = 0
    def enable_y_grid(en=True):
        if en:
            self.plot.ygrid.grid_line_width = 1
        else:
            self.plot.ygrid.grid_line_width = 0
    def enable_grid(en = True):
        self.enable_x_grid(en)
        self.enable_y_grid(en)

