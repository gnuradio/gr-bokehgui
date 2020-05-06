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
from bokehgui import bokeh_plot_config, utils


class vec_sink_c(bokeh_plot_config):
    """
    Python side implementation for vector sink for complex values.
    It creates a line plot on the frontend. It gets data from
    vec_sink_c_proc class and streams to the frontend plot.
    """

    def __init__(self, doc, plot_lst, process, is_message = False):
        super(vec_sink_c, self).__init__()

        self.doc = doc
        self.process = process
        self.plot_lst = plot_lst

        self.size = self.process.get_vlen()
        self.x_values = [i for i in range(self.size)]
        self.name = self.process.get_name()
        self.nconnections = self.process.get_nconnections()

        self.is_message = is_message
        self.plot = None
        self.stream = None
        self.max_hold = None
        self.legend_list = None
        self.lines_markers = None
        self.lines = None

    # def set_trigger_mode(self, trigger_mode, level, channel, tag_key):
    #     self.process.set_trigger_mode(trigger_mode, level, channel, tag_key)

    def initialize(self, legend_list = utils.default_labels_c,
                   update_time = 100):
        self.plot = figure(tools = utils.default_tools(), active_drag = 'ypan',
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

        self.stream = ColumnDataSource(data)

        self.lines = []
        self.lines_markers = []
        self.legend_list = legend_list[:]
        print(self.legend_list)
        for i in range(2 * nconn):
            self.lines.append(self.plot.line(x = 'x', y = 'y' + str(i),
                                             source = self.stream,
                                             line_color = 'blue',
                                             legend = self.legend_list[i]))
            self.lines_markers.append((None, None))

        self.add_custom_tools()

        # Add max-hold plot
        self.max_hold = None
        self.enable_max_hold(False)
        # max-hold plot done

        self.plot_lst.append(self)

        if self.name:
            self.set_title(self.name)

        self.doc.add_periodic_callback(self.update, update_time)

    def update(self):
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
            self.stream.stream(new_data, rollover = self.size)
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
        if self.max_hold is None:
            max_hold_source = ColumnDataSource(
                    data = dict(x = range(self.size),
                                y = [float("-inf")] * self.size))
            self.max_hold = self.plot.line(x = 'x', y = 'y',
                                           source = max_hold_source,
                                           line_color = 'green',
                                           line_dash = 'dotdash',
                                           line_alpha = 0)
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
            self.stream.js_on_change("streaming", callback)
        if en:
            self.max_hold.glyph.line_alpha = 1

        else:
            self.max_hold.glyph.line_alpha = 0
