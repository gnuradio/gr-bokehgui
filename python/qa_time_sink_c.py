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

import pmt
from bokehgui_swig import time_sink_c_proc
from gnuradio import blocks, gr, gr_unittest

class qa_time_sink_c(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        original = (
        1 + 1j, 2 + 2j, 3, 4 + 3j, 5 + 4j, 6, 7 + 1j, 8 + 9j, 1 - 1j, -1 + 1j,
        -2 - 1j, -3 + 10j,)
        expected_result = (0 + 0j,) + original

        src = blocks.vector_source_c(original, False, 1, [])
        dst = time_sink_c_proc(6, 32000, 'Test', 1)
        self.tb.connect(src, dst)

        self.tb.run()

        result_data = dst.get_plot_data()
        result_data1 = dst.get_plot_data()

        self.assertEqual(tuple([i.real for i in expected_result[0:6]]),
                         tuple(result_data[0]))
        self.assertEqual(tuple([i.imag for i in expected_result[0:6]]),
                         tuple(result_data[1]))
        self.assertEqual(tuple([i.real for i in expected_result[6:12]]),
                         tuple(result_data1[0]))
        self.assertEqual(tuple([i.imag for i in expected_result[6:12]]),
                         tuple(result_data1[1]))

        self.tb.stop()
        self.tb.wait()

    def test_002_t(self):
        src = blocks.vector_source_c(range(12), False, 1, [])

        tag = blocks.tags_strobe(gr.sizeof_gr_complex * 1, pmt.intern("TEST"),
                                 2, pmt.intern("strobe"))
        add = blocks.add_vcc(1)

        dst = time_sink_c_proc(6, 32000, 'Test', 1)

        self.tb.connect((src, 0), (add, 0))
        self.tb.connect((tag, 0), (add, 1))
        self.tb.connect((add, 0), (dst, 0))

        self.tb.run()

        result_data = dst.get_plot_data()
        tag_data = dst.get_tags()

        self.assertEqual(str(tag_data[0][0].key), "strobe")
        self.assertEqual(str(tag_data[0][0].value), "TEST")

        self.tb.stop()
        self.tb.wait()

if __name__ == '__main__':
    gr_unittest.run(qa_time_sink_c, "qa_time_sink_c.xml")
