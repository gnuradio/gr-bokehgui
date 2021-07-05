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

from bokeh.models import FuncTickFormatter
from bokeh.plotting import figure
from bokehgui import bokeh_plot_config, utils
from bokehgui.plots import WaterfallRenderer


class waterfall_sink_f(bokeh_plot_config):
    """
    Python side implementation for waterfall sink for float values.
    It creates a waterfall plot on the frontend. It gets data from
    waterfall_sink_f_proc class and streams to the frontend plot.
    """

    def __init__(self, plot_lst, proc, legend_list = utils.default_labels_f,
                   update_time = 100, values_range = (-200, 10),
                   time_per_sample = 0.1, number_of_samples = 200,
                   palette = 'Inferno', is_message = False):
        super(waterfall_sink_f, self).__init__()

        self.process = proc

        self.size = self.process.get_size()
        self.wintype = self.process.get_wintype()
        self.name = self.process.get_name()
        self.nconnections = self.process.get_nconnections()
        self.fc = self.process.get_center_freq()
        self.bw = self.process.get_bandwidth()

        self.is_message = is_message
        self.nrows = number_of_samples
        self.time_per_sample = time_per_sample
        self.frequency_range = None
        self.set_frequency_range(self.fc, self.bw, set_y_axis = False,
                                 notify_process = False)

        self.plot = None
        self.waterfall_renderer = None

        self.legend_list = legend_list[:]
        self.update_time = update_time
        self.values_range = values_range
        self.palette = palette

        plot_lst.append(self)

    def initialize(self, doc, plot_lst):

        plot = figure(tools = ['save', 'reset'],
                           y_range = [0, self.nrows],
                           x_range = [self.frequency_range[0],
                                      self.frequency_range[-1]],
                                      output_backend="webgl")
        plot.yaxis.formatter = FuncTickFormatter(code = """
                           return (%s - tick)*%s
                           """ % (self.nrows, self.time_per_sample))

        self.waterfall_renderer = []
        for i in range(self.nconnections):
            self.waterfall_renderer.append(
                WaterfallRenderer(palette = utils.PALETTES[self.palette],
                                  time_length = self.nrows,
                                  fft_length = self.size,
                                  min_value = self.values_range[0],
                                  max_value = self.values_range[-1]))
            plot.renderers.append(self.waterfall_renderer[i])

        self.plot = plot
        plot_lst.append(self)

        def callback():
            self.update( )

        doc.add_periodic_callback(callback, self.update_time)

    def update(self):
        output_items = self.process.get_plot_data()
        if len(output_items[0]) != 0:
            fc = self.process.get_center_freq()
            bw = self.process.get_bandwidth()
            fftsize = self.process.get_size()
            if self.fc != fc or self.bw != bw or self.size != fftsize:
                self.size = fftsize
                self.set_frequency_range(fc, bw, notify_process = False)

            if not self.is_message:
                for i in range(self.nconnections):
                    if self.waterfall_renderer[i].latest != list(output_items[i]):
                        self.waterfall_renderer[i].latest = list(output_items[i])
                    else:
                        self.waterfall_renderer[0].update = \
                            not self.waterfall_renderer[0].update

            else:
                self.time_per_sample = self.process.get_time_per_fft()
                self.plot.xaxis.formatter = FuncTickFormatter(code = """
                                   return tick*%s
                                   """ % self.time_per_sample)

                for i in range(self.nrows):
                    if self.waterfall_renderer[i].latest != list(output_items[i]):
                        self.waterfall_renderer[0].latest = output_items[i]
                    else:
                        self.waterfall_renderer[0].update = \
                            not self.waterfall_renderer[0].update
        return

    def set_frequency_range(self, fc, bw, set_y_axis = True,
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

        if set_y_axis:
            self.set_y_axis([fc - bw / 2, fc + bw / 2])

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

    def add_custom_tools(self):
        from bokeh.models import HoverTool, CrosshairTool
        hover = HoverTool(tooltips = [("x", "$x"), ("y", "$y")])
        crosshair = CrosshairTool()
        self.plot.add_tools(hover, crosshair)

    def set_color(self, palette):
        self.waterfall_renderers[0].palette = utils.PALETTES[palette]

    def disable_legend(self, en = True):
        # Pass the call. Waterfall plots does not support legends as of now
        pass
