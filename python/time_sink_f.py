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


class time_sink_f(bokeh_plot_config):
    """
    Python side implementation for time sink for float values.
    It creates a time series plot on the frontend. It gets data from
    time_sink_f_proc class and streams to the frontend plot.
    """

    def __init__(self, plot_lst, proc, log_x = False, log_y = False,
                   legend_list = utils.default_labels_f, update_time = 100, is_message = False):
        super(time_sink_f, self).__init__()

        self.process = proc

        self.size = self.process.get_size()
        self.samp_rate = self.process.get_samp_rate()
        self.name = self.process.get_name()
        self.nconnections = self.process.get_nconnections()
        self.is_message = is_message

        self.plot = None
        self.stream = None
        self.tags = None
        self.tag_stream = None
        self.tags_marker = None
        self.lines = None
        self.lines_markers = None
        self.legend_list = None

        self.log_x = log_x
        self.log_y = log_y
        self.legend_list = legend_list[:]
        self.update_time = update_time

        plot_lst.append(self)

    def set_trigger_mode(self, trigger_mode, trigger_slope, level, delay,
                         channel, tag_key):
        self.process.set_trigger_mode(trigger_mode, trigger_slope, level,
                                      delay, channel, tag_key)

    def initialize(self, doc, plot_lst):
        y_axis_type = 'log' if self.log_y else 'linear'
        x_axis_type = 'log' if self.log_x else 'linear'
        plot = figure(tools = utils.default_tools(), active_drag = 'ypan',
                           active_scroll = 'ywheel_zoom',
                           y_axis_type = y_axis_type,
                           x_axis_type = x_axis_type,
                           output_backend="webgl")
        data = dict()
        tag_data = dict()
        data['x'] = []

        if self.is_message:
            nconnection = 1
        else:
            nconnection = self.nconnections

        for i in range(nconnection):
            data['y' + str(i)] = []

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
            self.lines.append(plot.line(x = 'x', y = 'y' + str(i),
                              source = stream, line_color = self.colors[i],
                              line_width = self.widths[i], line_alpha=self.alphas[i],
                              legend_label = self.legend_list[i]))
            self.lines_markers.append((None, None))
            if self.styles[i] == 'None':
                self.lines[i].visible = False
            else:
                self.lines[i].glyph.line_dash = self.styles[i]
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
                                               fill_color = 'red', line_color = 'red')
                            )

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
            new_data['y' + str(i)] = output_items[i]

        if self.is_message:
            self.size = len(new_data['y0'])
            self.set_x_axis([0, self.size / self.samp_rate])
        new_data['x'] = self.values_x()

        new_tagged_data = {}
        max_tag_size = 0
        for i in range(self.nconnections):
            temp_x = []
            temp_y = []
            temp_tags = []
            for j in stream_tags[i].keys():
                temp_x.append(self.values_x()[j])
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

    def values_x(self):
        return [i / float(self.samp_rate) for i in range(self.size)]

    def set_samp_rate(self, samp_rate):
        self.process.set_samp_rate(samp_rate)
        self.samp_rate = samp_rate

    def set_nsamps(self, param, oldsize, newsize):
        if newsize != self.size:
            self.process.set_nsamps(newsize)
            self.size = newsize
        self.set_x_axis([0, self.size / self.samp_rate])

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
