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

from bokeh.models import ColumnDataSource, LabelSet
from bokeh.plotting import figure
from bokeh.models.ranges import Range1d
from bokehgui import bokeh_plot_config, utils


class const_sink_c(bokeh_plot_config):
    """
    Python side implementation for constellation sink for complex values.
    It creates a scatter plot on the frontend. It gets data from
    time_sink_c_proc class and streams to the frontend plot.
    """

    def __init__(self, plot_lst, process, legend_list = utils.default_labels_c,
                   update_time = 100, is_message = False):
        super(const_sink_c, self).__init__()

        self.process = process

        self.size = self.process.get_size()
        self.samp_rate = self.process.get_samp_rate()
        self.name = self.process.get_name()
        self.nconnections = self.process.get_nconnections()
        self.is_message = is_message

        self.plot = None
        self.stream = None
        self.tag_stream = None
        self.lines_markers = None
        self.tags_marker = None
        self.legend_list = None
        self.tags = None
        self.update_callback = None

        self.legend_list = legend_list[:]
        self.update_time = update_time

        plot_lst.append(self)


    def set_trigger_mode(self, trigger_mode, trigger_slope, level, delay,
                         channel, tag_key):
        self.process.set_trigger_mode(trigger_mode, trigger_slope, level,
                                      delay, channel, tag_key)

    def initialize(self, doc, plot_lst):
        tools = utils.default_tools()
        tools.remove('ypan')
        tools.remove('ywheel_zoom')
        tools.append('pan')
        tools.append('wheel_zoom')
        plot = figure(tools = tools, active_drag = 'pan',
                           active_scroll = 'wheel_zoom',
                           y_axis_type = 'linear', x_axis_type = 'linear',
                           output_backend="webgl")
        data = dict()

        tag_data = dict()

        if self.is_message:
            nconnection = 1
        else:
            nconnection = self.nconnections

        for i in range(nconnection):
            data['y' + str(i)] = []
            data['x' + str(i)] = []

            tag_data['x' + str(i)] = []
            tag_data['y' + str(i)] = []
            if not self.is_message:
                tag_data['tags' + str(i)] = []
        stream = ColumnDataSource(data)
        self.lines = []  #TODO: de self ize
        self.lines_markers = []
        if self.tags_enabled:
            tag_stream = ColumnDataSource(tag_data)
            if not self.is_message:
                self.tags = []
                self.tags_marker = []
        else:
            tag_stream = None
        for i in range(nconnection):
            self.lines_markers.append((
                plot.scatter(x = 'x' + str(i), y = 'y' + str(i),
                                  source = stream,
                                  legend = self.legend_list[i], ), 'o'))

            if self.tags_enabled:
                if not self.is_message:
                    self.tags.append(LabelSet(x = 'x' + str(i), y = 'y' + str(i),
                                              text = 'tags' + str(i),
                                              level = 'glyph', x_offset = -20,
                                              y_offset = 5,
                                              source = tag_stream,
                                              text_font_style = 'bold',
                                              text_font_size = '11pt',
                                              render_mode = 'canvas'))
                    self.tags_marker.append(
                            plot.triangle(x = 'x' + str(i), y = 'y' + str(i),
                                               source = tag_stream, size = 10,
                                               fill_color = 'red', ))
                    plot.add_layout(self.tags[i])

        if self.title_text is not None:
            plot.title.text = self.title_text
        if self.y_range is not None:
            plot.y_range = Range1d(self.y_range[0], self.y_range[1])
        if self.x_range is not None:
            plot.x_range = Range1d(self.x_range[0], self.x_range[1])
        if self.y_label is not None:
            plot.yaxis[0].axis_label = self.y_label
        if self.x_label is not None:
            plot.xaxis[0].axis_label = self.x_label
        plot.xgrid.visible = self.x_grid
        plot.ygrid.visible = self.y_grid
        if self.en_axis_labels:
            plot.xaxis[0].axis_label_text_color = '#000000'
            plot.yaxis[0].axis_label_text_color = '#000000'
        else:
            plot.xaxis[0].axis_label_text_color = '#FFFFFF'
            plot.yaxis[0].axis_label_text_color = '#FFFFFF'
        plot.legend[0].visible = self.en_legend
        plot.legend[0].click_policy = "hide"

        self.plot = plot
        self.stream = stream
        self.add_custom_tools()
        plot_lst.append(self)

        for i in range(nconnection):
            self.use_line_marker(i,self.markers[i])

        def callback():
            self.update(tag_stream, stream)

        doc.add_periodic_callback(callback, self.update_time)

    def update(self, tag_stream, stream):
        # Call to receive from buffers
        output_items = self.process.get_plot_data()
        if not self.is_message:
            tags = self.process.get_tags()
            stream_tags = []
            for i in range(self.nconnections):
                temp_stream_tags = {}
                for j in range(len(tags[i])):
                    temp_stream_tags[tags[i][j].offset] = str(
                            tags[i][j].key) + ":" + str(tags[i][j].value)

                stream_tags.append(temp_stream_tags)

        if len(output_items[0]) == 0:  # No output to send
            return

        new_data = dict()
        if self.is_message:
            nconnection = 1
        else:
            nconnection = self.nconnections

        for i in range(nconnection):
            new_data['x' + str(i)] = output_items[2 * i]
            new_data['y' + str(i)] = output_items[2 * i + 1]

        new_tagged_data = {}
        max_tag_size = 0
        for i in range(self.nconnections):
            temp_x = []
            temp_y = []
            temp_tags = []
            for j in stream_tags[i].keys():
                temp_x.append(new_data['x' + str(i)][j])
                temp_y.append(new_data['y' + str(i)][j])
                temp_tags.append(stream_tags[i][j])
            new_tagged_data['x' + str(i)] = temp_x
            new_tagged_data['y' + str(i)] = temp_y
            new_tagged_data['tags' + str(i)] = temp_tags
            if len(temp_x) > max_tag_size:
                max_tag_size = len(temp_x)

        stream.stream(new_data, rollover = self.size)
        if tag_stream is not None:
            tag_stream.stream(new_tagged_data, rollover = max_tag_size)
        return

    def set_samp_rate(self, samp_rate):
        self.process.set_samp_rate(samp_rate)
        self.samp_rate = samp_rate

    def set_nsamps(self, param, oldsize, newsize):
        if newsize != self.size:
            self.process.set_nsamps(newsize)
            self.size = newsize

    # def enable_tags(self, which = -1, en = True):
    #     if which == -1:
    #         for i in range(self.nconnections):
    #             self.enable_tags(i, en)
    #     else:
    #         if en:
    #             self.tags[which].text_color = 'black'
    #             self.tags_marker[which].visible = True
    #         else:
    #             self.tags[which].text_color = None
    #             self.tags_marker[which].visible = False
    def enable_tags(self, which = -1, en = True): #This does not allow for selective enable, but the interface doesn't allow it either
        self.tags_enabled = en

    def format_line(self, i, color, width, style, marker, alpha):
        self.set_line_color(i, color)
        self.set_line_width(i, width)
        self.set_line_marker(i, marker)
        self.set_line_alpha(i, alpha)

    def set_line_color(self, i, color):
        self.colors[i] = color
        if self.plot is not None:
            self.lines_markers[i][0].glyph.line_color = color


    def set_line_width(self, i, width):
        self.widths[i] = width
        if self.plot is not None:
            self.lines_markers[i][0].glyph.line_width = self.widths[i]

    def set_line_style(self, i, style):
        self.styles[i] = style
        if self.plot is not None:
            if style == 'None':
                self.lines[i].visible = False
                return
            self.lines[i].visible = True
            # solid, dashed, dotted, dotdash, dashdot
            self.lines[i].glyph.line_dash = self.styles[i]

    def set_line_marker(self, i, marker):
        self.markers[i] = marker
        if self.plot is not None:
            self.use_line_marker(i,self.markers[i])

    def set_line_alpha(self, i, alpha):
        self.alphas[i] = alpha
        if self.plot is not None:
            self.lines_markers[i][0].glyph.line_alpha = self.alphas[i]

    def use_line_marker(self, i, marker):
        if marker == 'None':
            self.lines_markers[i] = (None, None)
        if marker == '*':
            self.lines_markers[i] = (
            self.plot.asterisk(x = 'x' + str(i), y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), '*')
        if marker == 'o':
            self.lines_markers[i] = (
            self.plot.circle(x = 'x' + str(i), y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), 'o')
        if marker == 'o+':
            self.lines_markers[i] = (
            self.plot.circle_cross(x = 'x' + str(i), y = 'y' + str(i),
                    source = self.stream, legend = self.legend_list[i], ),
            'o+')
        if marker == '+':
            self.lines_markers[i] = (
            self.plot.cross(x = 'x' + str(i), y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), '+')
        if marker == 'd':
            self.lines_markers[i] = (
            self.plot.diamond(x = 'x' + str(i), y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), 'd')
        if marker == 'd+':
            self.lines_markers[i] = (
            self.plot.diamond_cross(x = 'x' + str(i), y = 'y' + str(i),
                    source = self.stream, legend = self.legend_list[i], ),
            'd+')
        if marker == 'v':
            self.lines_markers[i] = (
            self.plot.inverted_triangle(x = 'x' + str(i), y = 'y' + str(i),
                    source = self.stream, legend = self.legend_list[i], ), 'v')
        if marker == 's':
            self.lines_markers[i] = (
            self.plot.square(x = 'x' + str(i), y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), 's')
        if marker == 's+':
            self.lines_markers[i] = (
            self.plot.square_cross(x = 'x' + str(i), y = 'y' + str(i),
                    source = self.stream, legend = self.legend_list[i], ),
            's+')
        if marker == 'sx':
            self.lines_markers[i] = (
            self.plot.square_x(x = 'x' + str(i), y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), 'sx')
        if marker == '^':
            self.lines_markers[i] = (
            self.plot.triangle(x = 'x' + str(i), y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), '^')
        if marker == 'x':
            self.lines_markers[i] = (
            self.plot.x(x = 'x' + str(i), y = 'y' + str(i), source = self.stream,
                    legend = self.legend_list[i], ), 'x')


    def add_custom_tools(self):
        from bokeh.models import HoverTool, CrosshairTool
        hover = HoverTool(tooltips = [("x", "$x"), ("y", "$y")],
                          renderers = [item[0] for item in self.lines_markers])
        crosshair = CrosshairTool()
        self.plot.add_tools(hover, crosshair)
