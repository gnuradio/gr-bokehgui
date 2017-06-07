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
import pmt
from bokehgui import time_sink_c_proc
import numpy as np

class qa_time_sink_c (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        original = (1+1j,2+2j,3, 4+3j, 5+4j,6,
                    7+1j, 8+9j, 1-1j, -1+1j, -2-1j, -3+10j,
                    4-1j)
        src = blocks.vector_source_c(original, False, 1, [])

        expected_result = (tuple([i/32000.0 for i in range(len(original))]), original)

        dst = time_sink_c_proc(6, 32000, 'Test', 1)
        self.tb.connect(src, dst)
        self.tb.run()

        result_data = dst.get_plot_data()
        result_data1 = dst.get_plot_data()

        self.assertEqual(tuple(np.real(expected_result[1][0:6])), tuple(result_data[1])) # Check real data
        self.assertEqual(tuple(np.imag(expected_result[1][0:6])), tuple(result_data[2])) # Check imag data
        self.assertEqual(tuple(np.real(expected_result[1][6:12])), tuple(result_data1[1])) # Check real data
        self.assertEqual(tuple(np.imag(expected_result[1][6:12])), tuple(result_data1[2])) # Check imag data
        self.tb.stop()
        self.tb.wait()
        self.tearDown()

if __name__ == '__main__':
    gr_unittest.run(qa_time_sink_c, "qa_time_sink_c.xml")
