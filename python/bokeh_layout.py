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

from bokeh.layouts import column, row

import bokeh.layouts as bk_layouts
from bokeh.layouts import widgetbox, Spacer

class WidgetLayout:
    def __init__(self, widgetbox):
        self.widgetbox = widgetbox

    def set_layout(self, row, col, rowspan, colspan):
        self.layout = Rectangle(row, col, rowspan, colspan)
    def get_figure(self):
        return self.widgetbox

class Rectangle:
    def __init__(self, row, col, rowspan, colspan):
        self.row = row
        self.col = col
        self.height = colspan
        self.width = rowspan
        self.end_row = self.row + rowspan - 1
        self.end_col = self.col + colspan - 1

    def is_overlap(self, rect):
        if (self.row <= rect.end_row) and (self.end_row >= rect.row):
            if (self.col <= rect.end_col) and (self.end_col >= rect.col):
                return True
        return False

class Layout:
    # Kind of Tree
    def __init__(self, lst, is_root = True, min_row = 1, min_col = 1, max_row = float('Inf'), max_col = float('Inf')):
        self.list = lst # List of objects

        if is_root:
            self.min_row = float('Inf')
            self.min_col = float('Inf')
            self.max_row = 0
            self.max_col = 0
            for i in self.list:
                if self.max_row < i.layout.end_row: self.max_row = i.layout.end_row
                if self.max_col < i.layout.end_col: self.max_col = i.layout.end_col
                if self.min_row > i.layout.row: self.min_row = i.layout.row
                if self.min_col > i.layout.row: self.min_col = i.layout.col

            if not self.check():
                raise Exception("Overlapping placements of widgets are not allowed")
        else:
            self.min_row = min_row
            self.max_row = max_row
            self.max_col = max_col
            self.min_col = min_col

    def check(self):
        # check if rectangles are overlapping
        for i in self.list:
            for j in self.list:
                if i != j:
                    if i.layout.is_overlap(j.layout):
                        return False
        return True

    def evaluate(self, sizing_mode = "stretch_both"):
        if self.list == []:
            return Spacer()
        if len(self.list) == 1:
            figure = self.list[0].get_figure()
            if self.min_row == self.list[0].layout.row \
              and self.max_row == self.list[0].layout.end_row \
              and self.min_col == self.list[0].layout.col \
              and self.max_col == self.list[0].layout.end_col:
                  return figure

        list1 = []
        list2 = []
        # Return an object of "Row/Column" of Bokeh with 2 children of Layout object
        def if_hori_line_cut(i, max_val, lst):
            for j in lst:
                if (j.layout.row <= i) and (j.layout.end_row > i):
                    return True
            for j in lst:
                if (j.layout.row <= (i+1) and j.layout.end_row > (i+1)):
                    return False
            # Line can be passed from i and also from i+1
            temp_list = []
            for j in lst:
                if(j.layout.end_row <= i):
                    temp_list.append(j)
            temp_list1 = []
            for j in lst:
                if(j.layout.end_row <= (i+1)):
                    temp_list1.append(j)
            if temp_list == temp_list1 and temp_list == []:
                return True
            else:
                return False

        for i in xrange(self.min_row, self.max_row):
            if if_hori_line_cut(i, self.max_row, self.list):
                continue
            for j in self.list:
                if(j.layout.end_row <= i):
                    list1.append(j)
                else:
                    list2.append(j)
            layout1 = Layout(list1, False, min_row = self.min_row, min_col = self.min_col, max_row = i, max_col = self.max_col)
            layout2 = Layout(list2, False, min_row = i+1, min_col = self.min_col, max_row = self.max_row, max_col = self.max_col)
            return bk_layouts.column(layout1.evaluate(), layout2.evaluate(), sizing_mode=sizing_mode)

        def if_vert_line_cut(i, max_val, lst):
            for j in lst:
                if (j.layout.col <= i) and (j.layout.end_col > i):
                    return True
            for j in lst:
                if (j.layout.col <= i+1 and j.layout.end_col > i+1):
                    return False
            # Line can be passed from i and also from i+1
            temp_list = []
            for j in lst:
                if(j.layout.end_col <= i):
                    temp_list.append(j)
            temp_list1 = []
            for j in lst:
                if(j.layout.end_col <= (i+1)):
                    temp_list1.append(j)
            if temp_list == temp_list1 and temp_list == []:
                return True
            else:
                return False

        for i in xrange(self.min_col, self.max_col):
            if if_vert_line_cut(i, self.max_col, self.list):
                continue
            for j in self.list:
                if(j.layout.end_col <= i):
                    list1.append(j)
                else:
                    list2.append(j)
            layout1 = Layout(list1, False, min_row = self.min_row, min_col = self.min_col, max_row = self.max_row, max_col = i)
            layout2 = Layout(list2, False, min_row = self.min_row, min_col = i+1, max_row = self.max_row, max_col = self.max_col)
            return bk_layouts.row(layout1.evaluate(), layout2.evaluate(), sizing_mode=sizing_mode)

        raise Exception("In valid position of plots. Can't define Circular arrangements of plots")

def create_layout(lst, sizing_mode = "stretch_both"):
    # lst -> [obj] having obj.layout
    layout = Layout(lst)
    return layout.evaluate(sizing_mode = sizing_mode)
