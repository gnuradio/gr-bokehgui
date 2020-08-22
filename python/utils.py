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

import os
import subprocess
import bokeh.palettes as bp
import bokeh.application as ba
import bokeh.application as ba
import bokeh.application.handlers as bh
from bokeh.server.server import Server
import bokehgui

def default_tools():
    return ['box_zoom', 'ypan', 'ywheel_zoom', 'save', 'reset']

default_labels_f = ["Data {0}".format(i) for i in range(10)]
default_labels_c = ["Re{{Data {0}}}".format(i // 2) if i % 2 == 0
                    else "Im{{Data {0}}}".format(i // 2) for i in range(10)]




def run_server(tb):
    port = subprocess.check_output([os.path.abspath(
        os.path.dirname(__file__)) + "/scripts/check-port.sh"])


    # app = ba.Application(ba.handlers.ScriptHandler(os.path.abspath(os.path.dirname(__file__)) + "/plots/bokehgui.py"]))
    # server_proc = subprocess.Popen(["bokeh", "serve", "--port", str(int(port)),
    #                                 "--allow-websocket-origin=*",
    #                                 os.path.abspath(os.path.dirname(
    #                                     __file__)) + "/plots/bokehgui.py"])

    def make_doc(doc):
        doc.title = tb.name()
        plot_list = []
        widget_list = []
        for i in tb.plot_lst:
            i.initialize(doc, plot_list )
        print(plot_list)
        for i in tb.widget_lst:
            i.initialize(widget_list )
        if widget_list:
            input_t = bokehgui.bokeh_layout.widgetbox(widget_list)
            widgetbox = bokehgui.bokeh_layout.WidgetLayout(input_t)
            widgetbox.set_layout(*((0, 0)))
            list_obj = [widgetbox] + plot_list
        else:
            list_obj = plot_list
        layout_t = bokehgui.bokeh_layout.create_layout(list_obj, "fixed")
        print(layout_t)
        doc.add_root(layout_t)


    handler = bh.FunctionHandler(make_doc)
    app = ba.Application(handler)
    server = Server(app)
    server.run_until_shutdown()
    return #server_proc, str(int(port))

PALETTES = {
    'Inferno':bp.all_palettes['Inferno'][256],
    'Magma'  :bp.all_palettes['Magma'][256],
    'Plasma' :bp.all_palettes['Plasma'][256],
    'Viridis':bp.all_palettes['Viridis'][256],
    'Greys'  :bp.all_palettes['Greys'][256]
    }
