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

import bokeh.layouts as bk_layouts
from bokeh.layouts import Spacer, WidgetBox, widgetbox


class WidgetLayout:
    def __init__(self, widgetbox):
        self.widgetbox = widgetbox
        self.layout = None

    def set_layout(self, row, col, rowspan = 1, colspan = 1):
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
    def __init__(self, lst, is_root = True, min_row = 0, min_col = 0,
                 max_row = float('Inf'), max_col = float('Inf')):
        self.list = lst  # List of objects

        if is_root:
            self.min_row = 0
            self.min_col = 0
            self.max_row = 0
            self.max_col = 0
            for i in self.list:
                if self.max_row < i.layout.end_row:
                    self.max_row = i.layout.end_row
                if self.max_col < i.layout.end_col:
                    self.max_col = i.layout.end_col

            if not self.check():
                raise Exception(
                        "Overlapping placements of widgets are not allowed")
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

    def evaluate(self, sizing_mode = "fixed", height = 1000.0, width = 1000.0,
                 do_horizontal_cut = True, do_vertical_cut = True):
        if not self.list:
            a = Spacer()
            a.height = int(height)
            a.width = int(width)
            return a
        if len(self.list) == 1:
            figure = self.list[0].get_figure()
            if self.min_row == self.list[0].layout.row and \
                    self.max_row == self.list[0].layout.end_row and \
                    self.min_col == self.list[0].layout.col and \
                    self.max_col == self.list[0].layout.end_col:
                if isinstance(figure, WidgetBox):
                    figure.height = int(height)
                    figure.width = int(width)
                else:
                    figure.plot_height = int(height)
                    figure.plot_width = int(width)
                return figure

        if do_vertical_cut:
            def if_vert_line_cut(i, lst):
                for j in lst:
                    if j.layout.col <= i < j.layout.end_col:
                        return True
                for j in lst:
                    if j.layout.col <= i + 1 < j.layout.end_col:
                        return False
                # Line can be passed from i and also from i+1
                temp_list = []
                for j in lst:
                    if j.layout.end_col <= i:
                        temp_list.append(j)
                temp_list1 = []
                for j in lst:
                    if j.layout.end_col <= i + 1:
                        temp_list1.append(j)
                if temp_list == temp_list1 and temp_list == []:
                    return True
                else:
                    return False

            vertical_cuts = [-1]
            for i in  range(self.min_col, self.max_col):
                if if_vert_line_cut(i, self.list):
                    continue
                vertical_cuts.append(i)
            vertical_cuts.append(self.max_col)
            if len(vertical_cuts) != 0:
                layouti = []
                widthi = []
                listi = []
                for i in range(1, len(vertical_cuts)):
                    listi.append([])
                    for j in self.list:
                        if j.layout.end_col <= vertical_cuts[i] and \
                                    j.layout.col > vertical_cuts[i-1]:
                            listi[i-1].append(j)
                    if len(listi[i-1]):
                        layouti.append(Layout(listi[i - 1], False,
                                              min_row = self.min_row,
                                              min_col = vertical_cuts[
                                                            i - 1] + 1,
                                              max_row = self.max_row,
                                              max_col = vertical_cuts[i]))
                        widthi.append(float(width * (
                            vertical_cuts[i] - vertical_cuts[i - 1])) / (
                                          self.max_col - self.min_col + 1))
                return bk_layouts.row(children = [
                    layouti[k].evaluate(sizing_mode = sizing_mode,
                                        height = height, width = widthi[k],
                                        do_horizontal_cut = True,
                                        do_vertical_cut = False) for k in
                    range(len(layouti))], sizing_mode = sizing_mode,
                        height = int(height), width = int(width))

        if do_horizontal_cut:
            def if_hori_line_cut(i, lst):
                for j in lst:
                    if (j.layout.row <= i) and (j.layout.end_row > i):
                        return True
                for j in lst:
                    if (j.layout.row <= (i + 1) and j.layout.end_row > (
                                i + 1)):
                        return False
                # Line can be passed from i and also from i+1
                temp_list = []
                for j in lst:
                    if j.layout.end_row <= i:
                        temp_list.append(j)
                temp_list1 = []
                for j in lst:
                    if j.layout.end_row <= i + 1:
                        temp_list1.append(j)
                if temp_list == temp_list1 and temp_list == []:
                    return True
                else:
                    return False

            horizontal_cuts = [-1]
            for i in  range(self.min_row, self.max_row):
                if if_hori_line_cut(i, self.list):
                    continue
                horizontal_cuts.append(i)
            horizontal_cuts.append(self.max_row)
            if len(horizontal_cuts) != 0:
                layouti = []
                heighti = []
                listi = []
                for i in range(1, len(horizontal_cuts)):
                    listi.append([])
                    for j in self.list:
                        if (j.layout.end_row <= horizontal_cuts[i]) and \
                                        j.layout.row > horizontal_cuts[i-1]:
                            listi[i-1].append(j)
                    if len(listi[i-1]):
                        layouti.append(Layout(listi[i - 1], False,
                                              min_row = horizontal_cuts[i-1]+1,
                                              min_col = self.min_col,
                                              max_row = horizontal_cuts[i],
                                              max_col = self.max_col))
                        heighti.append(float(height * (
                            horizontal_cuts[i] - horizontal_cuts[i-1])) / (
                                           self.max_row - self.min_row + 1))
                return bk_layouts.column(children = [
                    layouti[k].evaluate(sizing_mode = sizing_mode,
                                        height = heighti[k], width = width,
                                        do_horizontal_cut = False,
                                        do_vertical_cut = True) for k in
                    range(len(layouti))], sizing_mode = sizing_mode,
                        height = int(height), width = int(width))
        raise Exception("In valid position of plots. Can't define Circular "
                        "arrangements of plots")


def create_layout(lst, sizing_mode = "fixed", window_size=(1000,1000)):
    # lst -> [obj] having obj.layout
    layout = Layout(lst)
    return layout.evaluate(sizing_mode = sizing_mode, height = window_size[0],
                           width = window_size[1])
