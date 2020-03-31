#
# Copyright 2017 Free Software Foundation, Inc.
#
# This application is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This application is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

# The presence of this file turns this directory into a Python package

'''
This is the GNU Radio BOKEHGUI module. It allows all the outputs and widgets
to work over the network. Basically it streams the data to frontend and a
frontend plotting library `Bokeh` plots the data. The widget callbacks are
handled by `Bokeh` and corresponding callback functions are defined in each
sinks.
'''
from __future__ import unicode_literals

# import swig generated symbols into the bokehgui namespace
try:
    # this might fail if the module is python-only
    from .bokehgui_swig import *
except ImportError:
    pass

# import any pure python here
from .bokeh_plot_config import bokeh_plot_config
from .checkbox import checkbox
from .button import button
from .const_sink_c import const_sink_c
from .freq_sink_c import freq_sink_c
from .freq_sink_f import freq_sink_f
from .label import label
from .radio_button import radiobutton
from .range_slider import range_slider
from .slider import slider
from .textbox import textbox
from .time_sink_c import time_sink_c
from .time_sink_f import time_sink_f
from .waterfall_sink_c import waterfall_sink_c
from .waterfall_sink_f import waterfall_sink_f
#
