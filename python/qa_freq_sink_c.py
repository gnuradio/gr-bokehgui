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

import numpy as np
from gnuradio import gr, gr_unittest
from gnuradio import blocks
from gnuradio import filter
from bokehgui import freq_sink_c_proc

class qa_freq_sink_c (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        original = (1+0j,)*100 + (0+1j,)*100 + (0+2j,)*100
        expected_result = [(-200,)*50 + (0,) + (-200,)*49,
                           (-200,)*50 + (0,) + (-200,)*49,
                           (-200,)*50 + (6.02,) + (-200,)*49,
                          ]

        src = blocks.vector_source_c(original, False, 1, [])

        dst = freq_sink_c_proc(100, filter.firdes.WIN_RECTANGULAR, 0, 15000, 'Test', 1)

        self.tb.connect(src, dst)
        self.tb.run()

        result_data = dst.get_plot_data()
        result_data1 = dst.get_plot_data()
        result_data2 = dst.get_plot_data()

        self.assertEqual(expected_result[0], tuple(round(x,2) for x in result_data[0]))
        self.assertEqual(expected_result[1], tuple(round(x,2) for x in result_data1[0]))
        self.assertEqual(expected_result[2], tuple(round(x,2) for x in result_data2[0]))

        self.tb.stop()
        self.tb.wait()

if __name__ == '__main__':
    gr_unittest.run(qa_freq_sink_c, "qa_freq_sink_c.xml")

