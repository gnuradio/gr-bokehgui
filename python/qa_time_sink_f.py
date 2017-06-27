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
                    18,)
        expected_result = original

        src = blocks.vector_source_f(original, False, 1, [])

        print "done"
        dst = time_sink_f_proc(6, 32000, 'Test', 1)

        self.tb.connect(src, dst)
        self.tb.run()
        print "done1"

        result_data = dst.get_plot_data()
        result_data1 = dst.get_plot_data()
        result_data2 = dst.get_plot_data()

        self.assertEqual(expected_result[0:6], tuple(result_data[0]))
        self.assertEqual(expected_result[6:12], tuple(result_data1[0]))
        self.assertEqual(expected_result[12:18], tuple(result_data2[0]))

        self.tb.stop()
        self.tb.wait()

#    def test_002_t (self):
#        src = blocks.vector_source_f(range(12), False, 1, [])
#
#        tag = blocks.tags_strobe(gr.sizeof_float*1, pmt.intern("TEST"), 2, pmt.intern("strobe"))
#        add = blocks.add_vff(1)
#
#        dst = time_sink_f_proc(6, 32000, 'Test', 1)
#
#        self.tb.connect((src,0), (add,0))
#        self.tb.connect((tag,0), (add,1))
#        self.tb.connect((add,0), (dst,0))
#
#        self.tb.run()
#
#        result_data = dst.get_plot_data()
#        tag_data = dst.get_tags()
#
#        self.assertEqual(str(tag_data[0][0].key), "strobe")
#        self.assertEqual(str(tag_data[0][0].value), "TEST")
#
#        self.tb.stop()
#        self.tb.wait()

if __name__ == '__main__':
    gr_unittest.run(qa_time_sink_f, "qa_time_sink_f.xml")
