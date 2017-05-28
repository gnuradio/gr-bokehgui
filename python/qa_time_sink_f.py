#!/usr/bin/env python
#
# Copyright 2011 Free Software Foundation, Inc.
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
from bokehgui import time_sink_f_proc
import numpy as np

class qa_time_sink_f (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        original = (1,2,3,4,5,6,
                    7,8,9,10,11,12,
                    13,14,15,16,17,
                    18,19,20,21,22,
                    23,24,25,26,27,
                    28,29,30,31,32)
        src = blocks.vector_source_f(original, False, 1, [])

        expected_result = (tuple([i/32000.0 for i in range(len(original))]), original)

        dst = time_sink_f_proc(6, 32000, 'Test', 1)
        self.tb.connect(src, dst)
        self.tb.run()
        result_data = dst.get_plot_data()
        result_data1 = dst.get_plot_data()
        result_data2 = dst.get_plot_data()

        self.assertEqual(expected_result[1][12:18], tuple(result_data[1]))
        self.assertEqual(expected_result[1][18:24], tuple(result_data1[1]))
        self.assertEqual(expected_result[1][24:30], tuple(result_data2[1]))
        self.tb.stop()
        self.tb.wait()

if __name__ == '__main__':
    gr_unittest.run(qa_time_sink_f, "qa_time_sink_f.xml")
