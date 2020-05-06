#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Top Block
# Generated: Thu Mar  2 00:11:20 2017
##################################################

if __name__ == '__main__':
    import ctypes
    import sys

    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

import functools
import signal
import time

import bokehgui
import pmt
from bokeh.client import push_session
from bokeh.plotting import curdoc
from gnuradio import analog, blocks, gr


class top_block(gr.top_block):
    def __init__(self, doc):
        gr.top_block.__init__(self, "Top Block")
        self.doc = doc
        self.widget_lst = []
        self.plot_lst = []

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 32000

        ##################################################
        # Blocks
        ##################################################
        self.bokehgui_time_sink_f_proc_0 = bokehgui.time_sink_f_proc(1024,
                                                                     samp_rate,
                                                                     'TimeSink',
                                                                     2)
        self.bokehgui_time_sink_f_0 = bokehgui.time_sink_f(self.doc,
                                                           self.plot_lst,
                                                           self.bokehgui_time_sink_f_proc_0)
        self.bokehgui_time_sink_f_0.set_trigger_mode(bokehgui.TRIG_MODE_FREE,
                                                     bokehgui.TRIG_SLOPE_POS,
                                                     0, 0, 0, "")
        self.bokehgui_time_sink_f_0.initialize(
            legend_list = ['data0', 'data1'])
        self.bokehgui_time_sink_f_0.set_layout(1, 1, 1, 1)

        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_float * 1,
                                                 samp_rate, True)
        self.blocks_throttle_1 = blocks.throttle(gr.sizeof_float * 1,
                                                 samp_rate, True)
        self.tagged_1 = blocks.tags_strobe(gr.sizeof_float * 1,
                                           pmt.intern("TEST"), 256,
                                           pmt.intern("strobe1"))
        self.tagged_0 = blocks.tags_strobe(gr.sizeof_float * 1,
                                           pmt.intern("TEST"), 256,
                                           pmt.intern("strobe2"))
        self.adder = blocks.add_vff(1)
        self.adder_1 = blocks.add_vff(1)
        self.analog_sig_source_x_0 = analog.sig_source_f(samp_rate,
                                                         analog.GR_COS_WAVE,
                                                         500, 3, 0)
        self.analog_sig_source_x_1 = analog.sig_source_f(samp_rate,
                                                         analog.GR_COS_WAVE,
                                                         100, 1, 0)

        self.bokehgui_time_sink_f_0.set_line_color(0, 'black')
        self.bokehgui_time_sink_f_0.set_line_color(1, 'red')
        self.bokehgui_time_sink_f_0.set_line_marker(0, '^')

        layout_t = bokehgui.bokeh_layout.create_layout(self.plot_lst)
        self.doc.add_root(layout_t)
        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_sig_source_x_0, 0), (self.adder, 0))
        self.connect((self.tagged_0, 0), (self.adder, 1))
        self.connect((self.analog_sig_source_x_1, 0), (self.adder_1, 0))
        self.connect((self.tagged_1, 0), (self.adder_1, 1))
        self.connect((self.adder, 0), (self.blocks_throttle_0, 0))
        self.connect((self.adder_1, 0), (self.blocks_throttle_1, 0))
        self.connect((self.blocks_throttle_0, 0),
                     (self.bokehgui_time_sink_f_proc_0, 0))
        self.connect((self.blocks_throttle_1, 0),
                     (self.bokehgui_time_sink_f_proc_0, 1))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate)
        self.bokehgui_time_sink_f_0.set_sample_rate(self.samp_rate)


def main(top_block_cls = top_block, options = None):
    serverProc, port = bokehgui.utils.create_server()

    def killProc(signum, frame, tb):
        tb.stop()
        tb.wait()
        serverProc.terminate()
        serverProc.kill()

    time.sleep(1)
    try:
        # Define the document instance
        doc = curdoc()
        session = push_session(doc, session_id = "test",
                               url = "http://localhost:" + port + "/bokehgui")
        # Create Top Block instance
        tb = top_block_cls(doc)
        try:
            tb.start()
            signal.signal(signal.SIGTERM, functools.partial(killProc, tb = tb))
            session.loop_until_closed()
        finally:
            print "Exiting the simulation. Stopping Bokeh Server"
            tb.stop()
            tb.wait()
    finally:
        serverProc.terminate()
        serverProc.kill()


if __name__ == '__main__':
    main()
