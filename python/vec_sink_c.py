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

from bokeh.models import ColumnDataSource, CustomJS
from bokeh.plotting import figure
from bokeh.models.ranges import Range1d
from bokehgui import bokeh_plot_config, utils


class vec_sink_c(bokeh_plot_config):
    """
    Python side implementation for vector sink for complex values.
    It creates a line plot on the frontend. It gets data from
    vec_sink_c_proc class and streams to the frontend plot.
    """

    def __init__(self, plot_lst, process, legend_list = utils.default_labels_c,
                   update_time = 100, is_message = False):
        super(vec_sink_c, self).__init__()

        self.process = process

        self.size = self.process.get_vlen()
        self.x_values = [i for i in range(self.size)]
        self.name = self.process.get_name()
        self.nconnections = self.process.get_nconnections()

        self.is_message = is_message
        self.plot = None
        self.stream = None
        self.max_hold_plot = None
        self.max_hold = False
        self.legend_list = None
        self.lines_markers = None
        self.lines = None
        self.legend_list = legend_list[:]
        self.update_time = update_time

        plot_lst.append(self)

    # def set_trigger_mode(self, trigger_mode, level, channel, tag_key):
    #     self.process.set_trigger_mode(trigger_mode, level, channel, tag_key)

    def initialize(self, doc, plot_lst):
        plot = figure(tools = utils.default_tools(), active_drag = 'ypan',
                           active_scroll = 'ywheel_zoom',
                           output_backend="webgl")
        data = dict()
        data['x'] = []
        if self.is_message:
            nconn = 1
        else:
            nconn = self.nconnections
        for i in range(2 * nconn):
            data['y' + str(i)] = []

        stream = ColumnDataSource(data)

        self.lines = []
        self.lines_markers = []
        for i in range(2 * nconn):
            self.lines.append(plot.line(x = 'x', y = 'y' + str(i),
                                             source = stream,
                                             line_color = self.colors[i],
                                             line_width = self.widths[i], line_alpha=self.alphas[i],
                                             legend_label = self.legend_list[i]))
            self.lines_markers.append((None, None))
            if self.styles[i] == 'None':
                self.lines[i].visible = False
            else:
                self.lines[i].glyph.line_dash = self.styles[i]

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

        # Add max-hold plot
        max_hold_source = ColumnDataSource(
                data = dict(x = range(self.size),
                            y = [float("-inf")] * self.size))
        self.max_hold_plot = plot.line(x = 'x', y = 'y',
                                       source = max_hold_source,
                                       line_color = 'green',
                                       line_dash = 'dotdash',
                                       legend_label= 'Max')
        callback = CustomJS(args = dict(max_hold_source = max_hold_source),
                            code = """
                        var no_of_elem = cb_obj.data.x.length;
                        var data = cb_obj.data;
                        nconn = Object.getOwnPropertyNames(data).length -1;
                        var max_data = max_hold_source.data;
                        max_data.x = cb_obj.data.x;

                        for(n = 0; n < nconn; n++) {
                               for (i = 0; i < no_of_elem; i++) {
                                   if(max_data['y'][i] < data['y'+n][i]) {
                                       max_data['y'][i] = data['y'+n][i]
                                   }
                               }
                        }
                        max_hold_source.change.emit();
                        """)
        stream.js_on_change("streaming", callback)
        self.max_hold_plot.visible = self.max_hold
        # max-hold plot done

        plot_lst.append(self)

        def callback():
            self.update(stream)

        doc.add_periodic_callback(callback, self.update_time)

    def update(self, stream):
        # Call to receive from buffers
        output_items = self.process.get_plot_data()
        # print("updating: {}, should be of size {}".format(output_items, output_items.shape))
        if len(output_items[0]) != 0:
            # print("vector to plot!!!!")
            # vec_len = self.process.get_size()
            # if  self.size != vec_len:
            #     self.size = vec_len

            new_data = dict()
            for i in range(2* self.nconnections): # 2 channels per input stream (complex)
                new_data['y' + str(i)] = output_items[i]
            new_data['x'] = self.x_values
            stream.stream(new_data, rollover = self.size)
        return

    def set_x_values(self, values):
        if len(values) != self.size:
            print("Error: x values has not the size of the input vector")
            return
        self.x_values = values
    # def set_x_values(self, start, step):
    #     pass
        # self.x_values = values


    def enable_max_hold(self, en = True):
        self.max_hold = en
        if self.max_hold_plot:
            self.max_hold_plot.visible = self.max_hold
