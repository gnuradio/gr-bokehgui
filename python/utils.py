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

import subprocess, os

def default_tools():
    return [
         'box_zoom',
         'ypan',
         'ywheel_zoom',
         'save',
         'reset',
         'resize'
        ]

default_labels_f = ["Data {0}".format(i) for i in range(10)]
default_labels_c = ["Re{{Data {0}}}".format(i/2) if i%2 == 0 else "Im{{Data {0}}}".format(i/2) for i in range(10)]

def create_server():
    port = subprocess.check_output([os.path.abspath(os.path.dirname(__file__)) + "/scripts/start-server.sh"])
    server_proc = subprocess.Popen(["bokeh", "serve", "--port", str(port[:-1]), "--allow-websocket-origin=*", os.path.abspath(os.path.dirname(__file__))+"/plots/bokehgui.py"])
    return server_proc, str(port[:-1])

import bokeh.palettes as bp
PALETTES = {
            'Inferno': bp.all_palettes['Inferno'][256],
            'Magma': bp.all_palettes['Magma'][256],
            'Plasma': bp.all_palettes['Plasma'][256],
            'Viridis': bp.all_palettes['Viridis'][256],
            'Greys': bp.all_palettes['Greys'][256]
            }

