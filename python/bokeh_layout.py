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

class Rectangle:
    def __init__(self, row, col, rowspan, colspan):
        self.row = row
        self.col = col
        self.height = colspan
        self.width = rowspan
        self.end_row = self.row + rowspan - 1
        self.end_col = self.col + colspan - 1

    def is_overlap(self, rect):
        if (self.row < rect.end_row) and (self.end_row > rect.row):
            return True
        if (self.col < rect.end_col) and (self.end_col > rect.col):
            return True
        else:
            return False

class Layout:
    # Kind of Tree
    def __init__(self, lst, do_check):
        self.list = lst # List of rectangles

        self.min_row = float('Inf')
        self.min_col = float('Inf')
        self.max_row = 0
        self.max_col = 0
        for i in self.list:
            if self.max_row < i.end_row: self.max_row = i.end_row
            if self.max_col < i.end_col: self.max_col = i.end_col
            if self.min_row > i.row: self.min_row = i.row
            if self.min_col > i.row: self.min_col = i.col
        if do_check:
            if not self.check():
                raise Exception("Overlapping placements of widgets are not allowed")


    def check(self):
        # check if rectangles are overlapping
        for i in self.list:
            for j in self.list:
                if i != j:
                    if i.overlap(j):
                        return False
        return True

    def evaluate(self):
        # Return an object of "Row/Column" of Bokeh with 2 children of Layout object
        for i in xrange(self.min_row, self.max_row):
            def if_hori_line_cut(i):
                for j in self.list:
                    if (j.row <= i) and (j.end_row > i):
                        return True
                return False
            if if_hori_line_cut(i):
                continue
            for j in self.list:
                if(j.end_row <= i):
                    list1.append(j)
                else:
                    list2.append(j)
            layout1 = Layout(self, list1, false)
            layout2 = Layout(self, list2, false)
            return column(layout1.evaluate(), layout2.evaluate())

        for i in xrange(self.min_col, self.max_col):
            def if_vert_line_cut(i):
                for j in self.lst:
                    if (j.col <= i) and (j.end_col > i):
                        return True
                return False
            if if_vert_line_cut(i):
                continue
            for j in self.list:
                if(j.end_col <= i):
                    list1.append(j)
                else:
                    list2.append(j)
            layout1 = Layout(self, list1, false)
            layout2 = Layout(self, list2, false)
            return row(layout1.evaluate(), layout2.evaluate())

        return Exception("In valid position of plots. Can't define Circular arrangements of plots")
