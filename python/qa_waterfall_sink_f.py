#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright 2011-2013,2015 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
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
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
from gnuradio import filter
from bokehgui import waterfall_sink_f_proc

class qa_waterfall_sink_f (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        original = (1,)*100 + (-1,)*100 + (0,)*50 + (10,) + (0,)*49
        expected_result = [(-200,)*50 + (0,) + (-200,)*49,
                           (-20,)*100]

        src = blocks.vector_source_f(original, False, 1, [])

        dst = waterfall_sink_f_proc(100, filter.firdes.WIN_RECTANGULAR, 0, 15000, 'Test', 1)

        self.tb.connect(src, dst)
        self.tb.run()

        result_data = dst.get_plot_data()
        result_data1 = dst.get_plot_data()

        self.assertEqual(expected_result[0], tuple(result_data[0]))
        self.assertEqual(expected_result[0], tuple(result_data1[0]))

        self.tb.stop()
        self.tb.wait()

if __name__ == '__main__':
    gr_unittest.run(qa_waterfall_sink_f, "qa_waterfall_sink_f.xml")
