# Copyright 2017 Free Software Foundation, Inc.
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

from .bokeh_layout import Rectangle

class bokeh_plot_config(object):
    def __init__(self):
        self.plot = None
        self.stream = None
        self.title_text = None
        self.y_range = None
        self.x_range = None
        self.y_label = None
        self.x_label = None
        self.x_grid = True
        self.y_grid = True
        self.en_axis_labels = True
        self.en_legend = True
        self.colors = ["blue", "red", "green", "black", "cyan",
                  "magenta", "yellow", "blue", "blue", "blue"]
        self.widths = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        self.styles = ["solid", "solid", "solid", "solid", "solid",
                  "solid", "solid", "solid", "solid", "solid"]
        self.markers = [None, None, None, None, None,
                  None, None, None, None, None]
        self.alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

    def get_figure(self):
        return self.plot

    def set_layout(self, row, col, rowspan = 1, colspan = 1):
        self.layout = Rectangle(row, col, rowspan, colspan)

    def add_custom_tools(self):
        from bokeh.models import HoverTool, CrosshairTool
        hover = HoverTool(tooltips = [("x", "$x"), ("y", "$y")],
                          mode = 'vline', renderers = self.lines)
        crosshair = CrosshairTool()
        self.plot.add_tools(hover, crosshair)

    def set_title(self, name):
        self.title_text = name
        if self.plot is not None:
            plot.title.text = self.title_text

    def get_title(self):
        return self.title_text

    def set_y_axis(self, lst):
        assert (lst[0] < lst[1])
        self.y_range = lst
        if self.plot is not None:
            from bokeh.models.ranges import Range1d
            plot.y_range = Range1d(self.y_range[0], self.y_range[1])

    def set_x_axis(self, lst):
        assert (lst[0] < lst[1])
        self.x_range = lst
        if self.plot is not None:
            from bokeh.models.ranges import Range1d
            plot.x_range = Range1d(self.x_range[0], self.x_range[1])

    def set_x_label(self, xlabel):
        self.x_label = xlabel
        if self.plot is not None:
            plot.xaxis[0].axis_label = self.x_label

    def set_y_label(self, ylabel):
        self.y_label = ylabel
        if self.plot is not None:
            plot.yaxis[0].axis_label = self.y_label

    def format_line(self, i, color, width, style, marker, alpha):
        self.set_line_color(i, color)
        self.set_line_width(i, width)
        self.set_line_style(i, style)
        self.set_line_marker(i, marker)
        self.set_line_alpha(i, alpha)

    def set_line_color(self, i, color):
        self.colors[i] = color
        if self.plot is not None:
            self.lines[i].glyph.line_color = color

    def get_line_color(self, i):
        return self.colors[i]

    def set_line_width(self, i, width):
        self.widths[i] = width
        if self.plot is not None:
            self.lines[i].glyph.line_width = self.widths[i]

    def get_line_width(self, i):
        return self.widths[i]

    def set_line_style(self, i, style):
        self.styles[i] = style
        if self.plot is not None:
            if style == 'None':
                self.lines[i].visible = False
                return
            self.lines[i].visible = True
            # solid, dashed, dotted, dotdash, dashdot
            self.lines[i].glyph.line_dash = self.styles[i]

    def get_line_style(self, i):
        # if not self.lines[i].visible:
        #     return
        # solid, dashed, dotted, dotdash, dashdot
        return self.styles[i]

    def use_line_marker(self, i, marker):
        if marker == 'None':
            self.lines_markers[i] = (None, None)
        if marker == '*':
            self.lines_markers[i] = (
            self.plot.asterisk(x = 'x', y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), '*')
        if marker == 'o':
            self.lines_markers[i] = (
            self.plot.circle(x = 'x', y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), 'o')
        if marker == 'o+':
            self.lines_markers[i] = (
            self.plot.circle_cross(x = 'x', y = 'y' + str(i),
                    source = self.stream, legend = self.legend_list[i], ),
            'o+')
        if marker == '+':
            self.lines_markers[i] = (
            self.plot.cross(x = 'x', y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), '+')
        if marker == 'd':
            self.lines_markers[i] = (
            self.plot.diamond(x = 'x', y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), 'd')
        if marker == 'd+':
            self.lines_markers[i] = (
            self.plot.diamond_cross(x = 'x', y = 'y' + str(i),
                    source = self.stream, legend = self.legend_list[i], ),
            'd+')
        if marker == 'v':
            self.lines_markers[i] = (
            self.plot.inverted_triangle(x = 'x', y = 'y' + str(i),
                    source = self.stream, legend = self.legend_list[i], ), 'v')
        if marker == 's':
            self.lines_markers[i] = (
            self.plot.square(x = 'x', y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), 's')
        if marker == 's+':
            self.lines_markers[i] = (
            self.plot.square_cross(x = 'x', y = 'y' + str(i),
                    source = self.stream, legend = self.legend_list[i], ),
            's+')
        if marker == 'sx':
            self.lines_markers[i] = (
            self.plot.square_x(x = 'x', y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), 'sx')
        if marker == '^':
            self.lines_markers[i] = (
            self.plot.triangle(x = 'x', y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), '^')
        if marker == 'x':
            self.lines_markers[i] = (
            self.plot.x(x = 'x', y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), 'x')

    def set_line_marker(self, i, marker):
        self.markers[i] = marker
        if self.plot is not None:
            self.use_line_marker(i,self.markers[i])

    def get_line_marker(self, i):
        return self.markers[i]

    def set_line_alpha(self, i, alpha):
        self.alphas[i] = alpha
        if self.plot is not None:
            self.lines[i].glyph.line_alpha = self.alphas[i]

    def get_line_alpha(self, i):
        return self.alphas[i]

    def enable_x_grid(self, en = True):
        self.x_grid = en
        if self.plot is not None:
            self.plot.xgrid.visible = self.x_grid

    def enable_y_grid(self, en = True):
        self.y_grid = en
        if self.plot is not None:
            self.plot.ygrid.visible = self.y_grid

    def enable_grid(self, en = True):
        self.enable_x_grid(en)
        self.enable_y_grid(en)

    def enable_axis_labels(self, en = True):
        self.en_axis_labels = en
        if self.plot is not None:
            if self.en_axis_labels:
                self.plot.xaxis[0].axis_label_text_color = '#000000'
                self.plot.yaxis[0].axis_label_text_color = '#000000'
            else:
                self.plot.xaxis[0].axis_label_text_color = '#FFFFFF'
                self.plot.yaxis[0].axis_label_text_color = '#FFFFFF'

    def enable_legend(self, en = True):
        self.en_legend = en
        if self.plot is not None:
            self.plot.legend[0].visible = self.en_legend
            self.plot.legend[0].click_policy = "hide"
