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

from bokeh.models import ColumnDataSource, CustomJS, CustomAction
from bokeh.plotting import figure
from bokeh.models.ranges import Range1d
from bokehgui import bokeh_plot_config, utils
import numpy as np


class number_sink(bokeh_plot_config):
    """
    Python side implementation for number sink for float values.
    It creates a bar plot on the frontend. It gets data from
    vec_sink_f_proc class and streams to the frontend plot.
    """

    def __init__(
        self,
        plot_lst,
        process,
        legend_list=utils.default_labels_f,
        update_time=100,
        is_message=False,
        is_complex=False
    ):
        super(number_sink, self).__init__()

        self.process = process

        self.size = self.process.get_vlen()
        self.x_values = [i for i in range(self.size)]
        self.name = self.process.get_name()
        self.nconnections = self.process.get_nconnections()
        if is_complex:
            self.nconnections *= 2
        self.is_complex = is_complex

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


    def initialize(self, doc, plot_lst):
        
        plot = figure(
            tools=utils.default_tools(),
            active_drag="ypan",
            active_scroll="ywheel_zoom",
            output_backend="canvas",
            title=self.name,
            x_range=self.legend_list
        )
        data = dict()

        data["x"] = self.legend_list[:self.nconnections]
        data["top"] = list(np.zeros(self.nconnections))
        data["colors"] = self.colors[:self.nconnections]
        data["alphas"] = self.alphas[:self.nconnections]
        data["widths"] = self.widths[:self.nconnections]

        stream = ColumnDataSource(data)

        self.lines = []
        self.lines_markers = []
        plot.vbar(x='x',
                  top='top',
                  width='widths',
                  bottom=-np.inf,
                  color="colors",
                  alpha="alphas",
                  legend_field="x",
                  source=stream
                  )

        if self.title_text is not None:
            plot.title.text = self.title_text
        if self.y_range is not None:
            plot.y_range = Range1d(self.y_range[0], self.y_range[1])
        if self.y_label is not None:
            plot.yaxis[0].axis_label = self.y_label
        plot.xgrid.visible = self.x_grid
        plot.ygrid.visible = self.y_grid
        if self.en_axis_labels:
            plot.xaxis.axis_label_text_color = "#000000"
            plot.yaxis.axis_label_text_color = "#000000"
        else:
            plot.xaxis.axis_label_text_color = "#FFFFFF"
            plot.yaxis.axis_label_text_color = "#FFFFFF"
        plot.legend.visible = self.en_legend
        plot.legend.click_policy = "hide"

        self.plot = plot
        self.stream = stream
        self.add_custom_tools()

        # Add max-hold plot
        max_hold_source = ColumnDataSource(
            data=dict(
                x=self.legend_list[:self.nconnections], top=np.full(self.nconnections, -np.inf, dtype=np.float32), widths=self.widths[:self.nconnections]
            )
        )
        self.max_hold_plot = plot.rect(
            x="x",
            y="top",
            source=max_hold_source,
            width="widths",
            height={'value':2, 'units':"screen"},
            color="green",
            alpha=0.8,
            legend_label="Max",
        )
        callback_js = CustomJS(
            args=dict(max_hold_source=max_hold_source),
            code="""
                        var no_of_elem = cb_obj.data.x.length;
                        var data = cb_obj.data;
                        var max_data = max_hold_source.data;
                        max_data.x = cb_obj.data.x;
                        max_data.widths = cb_obj.data.widths;
                        for(let i = 0; i < no_of_elem; i++) {
                            if(max_data['top'][i] < data['top'][i]) {
                                max_data['top'][i] = data['top'][i]
                            }
                        }
                        max_hold_source.change.emit();
                        """,
        )
        stream.js_on_change("data", callback_js)
        self.max_hold_plot.visible = self.max_hold

        reset_action = CustomAction(
            # icon=r"/path/to/icon.png", # TODO: Put some actual icon, and understand how to specify the path
            callback = CustomJS(
                args=dict(max_hold_source=max_hold_source),
                code='''
                    var max_data = max_hold_source.data;
                    var no_of_elem = max_hold_source.data.top.length;

                    for (let i = 0; i < no_of_elem; i++) {
                        max_data['top'][i] = -Infinity
                    }
                    max_hold_source.change.emit();
                '''
            ),
            description='Reset Max'
        )
        plot.add_tools(reset_action)
        # max-hold plot done

        plot_lst.append(self)

        def callback():
            self.update(stream)

        doc.add_periodic_callback(callback, self.update_time)

    def update(self, stream):
        # Call to receive from buffers
        output_items = self.process.get_plot_data()
        if len(output_items[0]) != 0:
            stream.data["top"] = list(output_items[:,0])
        return

    def set_x_values(self, values):
        if len(values) != self.size:
            print(
                f"Error: x values has not the size ({len(values)}) of the input vector ({self.size})"
            )
            return
        self.x_values = values

    def enable_max_hold(self, en=True):
        self.max_hold = en
        if self.max_hold_plot:
            self.max_hold_plot.visible = self.max_hold

    def format_line(self, i, color, width, alpha):
        self.set_line_color(i, color)
        self.set_line_width(i, width)
        self.set_line_alpha(i, alpha)
