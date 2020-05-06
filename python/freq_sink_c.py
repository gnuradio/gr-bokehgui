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


class freq_sink_c(bokeh_plot_config):
    """
    Python side implementation for frequency sink for complex values.
    It creates a line series plot on the frontend. It gets data from
    freq_sink_c_proc class and streams to the frontend plot.
    """

    def __init__(self, doc, plot_lst, proc, is_message = False):
        super(freq_sink_c, self).__init__()

        self.doc = doc
        self.process = proc
        self.plot_lst = plot_lst

        self.size = self.process.get_size()
        self.wintype = self.process.get_wintype()
        self.name = self.process.get_name()
        self.nconnections = self.process.get_nconnections()
        self.fc = self.process.get_center_freq()
        self.bw = self.process.get_bandwidth()

        self.is_message = is_message
        self.frequency_range = None
        self.set_frequency_range(self.fc, self.bw, set_x_axis = False,
                                 notify_process = False)

        self.plot = None
        self.stream = None
        self.lines = None
        self.lines_markers = None
        self.legend_list = None
        self.max_hold = None

    def set_trigger_mode(self, trigger_mode, level, channel, tag_key):
        self.process.set_trigger_mode(trigger_mode, level, channel, tag_key)

    def initialize(self, legend_list = utils.default_labels_f,
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
        for i in range(nconn):
            data['y' + str(i)] = []

        self.stream = ColumnDataSource(data)

        self.lines = []
        self.lines_markers = []
        self.legend_list = legend_list[:]
        for i in range(self.nconnections):
            self.lines.append(
                self.plot.line(x = 'x', y = 'y' + str(i), source = self.stream,
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

        # First call to check if BW and FC is not changed
        # Through some direct message port to the block
        output_items = self.process.get_plot_data()
        if len(output_items[0]) != 0:
            fc = self.process.get_center_freq()
            bw = self.process.get_bandwidth()
            fftsize = self.process.get_size()
            if self.fc != fc or self.bw != bw or self.size != fftsize:
                self.size = fftsize
                self.set_frequency_range(fc, bw, notify_process = False)

            new_data = dict()
            for i in range(self.nconnections + 1):
                if (not self.is_message) and i == self.nconnections:
                    continue
                new_data['y' + str(i)] = output_items[i]
            new_data['x'] = self.frequency_range
            self.stream.stream(new_data, rollover = self.size)
        return

    def set_frequency_range(self, fc, bw, set_x_axis = True,
                            notify_process = True):
        self.fc = fc
        self.bw = bw
        if notify_process:
            self.process.set_frequency_range(fc, bw)

        step = bw / self.size

        self.frequency_range = [0] * self.size
        for i in range(self.size // 2):
            self.frequency_range[i] = fc - step * (self.size // 2 - i)
        self.frequency_range[(self.size + 1) // 2] = self.fc
        for i in range((self.size - 1) // 2):
            self.frequency_range[i + 1 + (self.size + 1) // 2] = fc + step * i

        if set_x_axis:
            self.set_x_axis([fc - bw / 2, fc + bw / 2])

    def set_center_freq(self, fc):
        if self.fc != fc:
            self.fc = fc
            self.set_frequency_range(fc, self.bw)

    def set_bandwidth(self, bw):
        if self.bw != bw:
            self.bw = bw
            self.set_frequency_range(self.fc, bw)

    def set_fft_window(self, wintype):
        if not self.wintype == wintype:
            self.wintype = wintype
            self.process.set_fft_window(wintype)

    def set_fft_size(self, fftsize):
        if fftsize < 16 or fftsize > 16384:
            raise ValueError("FreqSink: FFT Size must be between 16 to 16384")
        self.size = fftsize
        self.process.fftresize(fftsize)

    def enable_max_hold(self, en = True):
        if self.max_hold is None:
            max_hold_source = ColumnDataSource(
                    data = dict(x = range(self.size), y = [-1000] * self.size))
            self.max_hold = self.plot.line(x = 'x', y = 'y',
                                           source = max_hold_source,
                                           line_color = 'green',
                                           line_dash = 'dotdash',
                                           line_alpha = 0)
            callback = CustomJS(
                    args = dict(max_hold_source = self.max_hold.data_source),
                    code = """
                        var no_of_elem = cb_obj.data.x.length;
                        var data = cb_obj.data;
                        nconn = Object.getOwnPropertyNames(data).length - 1;
                        var max_data = max_hold_source.data;
                        max_data.x = cb_obj.data.x;

                        for(n = 0; n < nconn; n++) {
                            for (i = 0; i < no_of_elem; i++) {
                                if(max_data['y'][i] < data['y'+n][i]) {
                                    max_data['y'][i] = data['y'+n][i];
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
