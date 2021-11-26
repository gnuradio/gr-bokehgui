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

from bokeh.models.widgets import Slider

class slider():
    def __init__(self, widget_lst, label, start, end, step, callback_throttle,
                 default):
        self.label = label
        self.start = start
        self.end = end
        self.step = step
        self.callback_throttle = callback_throttle
        self.default = default
        self.slider = None
        self.callback = None
        # self.initialize(label, start, end, step, callback_throttle, default)
        widget_lst.append(self)

    def initialize(self, widget_lst):
        self.slider = Slider(start = self.start, end = self.end, value = self.default,
                             step = self.step, title = self.label)
        widget_lst.append(self.slider)
        if self.callback is not None:
            self.slider.on_change('value', self.callback)

    def add_callback(self, callback):
        self.callback = callback
        if self.slider is not None:
            self.slider.on_change('value', self.callback) #May not keep that one

    def set_value(self, value):
        if self.slider is not None:
            self.slider.value = value
        self.default = value
