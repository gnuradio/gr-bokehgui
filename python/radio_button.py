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

from bokeh.models.widgets import RadioGroup

class radiobutton():
    def __init__(self, widget_lst, default_value, label, inline = True):
        self.widget_lst = widget_lst
        self.initialize(default_value, label, inline)

    def initialize(self, default_value, label, inline):
        self.radiobutton = RadioGroup(active = default_value, labels = label, inline = inline)
        self.widget_lst.append(self.radiobutton)

    def add_callback(self, callback):
        self.radiobutton.on_click(callback)
