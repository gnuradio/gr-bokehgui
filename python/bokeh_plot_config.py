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

from bokeh.models import LabelSet, Legend
from .bokeh_layout import Rectangle

class bokeh_plot_config(object):
    def __init__(self):
        self.plot = None
        self.stream = None

    def get_figure(self):
        return self.plot

    def set_layout(self, row, col, rowspan, colspan):
        self.layout = Rectangle(row, col, rowspan, colspan)

    def add_custom_tools(self):
        from bokeh.models import HoverTool, CrosshairTool
        hover = HoverTool(tooltips = [("x", "$x"),
                                      ("y", "$y")],
                          mode = 'vline',
                          renderers = self.lines)
        crosshair = CrosshairTool()
        self.plot.add_tools(hover, crosshair)

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

    def format_line(self, i, color, width, style, marker, alpha):
        self.set_line_color(i, color)
        self.set_line_width(i, width)
        self.set_line_style(i, style)
        self.set_line_marker(i, marker)
        self.set_line_alpha(i, alpha)

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
        if not self.lines[i].visible:
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
            self.plot.xaxis[0].axis_label_text_color = '#000000'
            self.plot.yaxis[0].axis_label_text_color = '#000000'
        else:
            self.plot.xaxis[0].axis_label_text_color = '#FFFFFF'
            self.plot.yaxis[0].axis_label_text_color = '#FFFFFF'

    def disable_legend(self, en=True):
        if en:
            self.plot.legend[0].visible = False
        else:
            self.plot.legend[0].visible = True
    # def set_size(self, height, width);

