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

import subprocess, time

from gnuradio import analog
from gnuradio import blocks
from gnuradio import gr
from gnuradio.filter import firdes

from bokeh.client import push_session
from bokeh.plotting import curdoc
from bokeh.layouts import column
import bokehgui

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
        self.bokehgui_time_sink_c_proc_0 = bokehgui.time_sink_c_proc(200, samp_rate, 'TimeSink', 2)
        self.bokehgui_time_sink_c_0 = bokehgui.time_sink_c(self.doc, self.plot_lst, self.bokehgui_time_sink_c_proc_0)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate,True)
        self.blocks_throttle_1 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate,True)
        self.analog_sig_source_x_0 = analog.sig_source_c(samp_rate, analog.GR_COS_WAVE, 300, 3, 0)
        self.analog_sig_source_x_1 = analog.sig_source_c(samp_rate, analog.GR_COS_WAVE, 100, 2, 0)

        ##################################################
        # Customizing the plot
        ##################################################
        self.bokehgui_time_sink_c_0.initialize(legend_list = ['Re{data0}',
                                                              'Im{data0}',
                                                              'Re{data1}',
                                                              'Im{data1}'],
                                               update_time = 100
                                               )
        self.bokehgui_time_sink_c_0.set_x_label('Time (s)')
        self.bokehgui_time_sink_c_0.set_y_label('Value')
        self.bokehgui_time_sink_c_0.set_line_color(0, 'black')
        self.bokehgui_time_sink_c_0.set_line_color(1, 'darkred')
        self.bokehgui_time_sink_c_0.set_line_color(2, 'green')
        self.bokehgui_time_sink_c_0.set_line_style(1, 'dashed')
        self.bokehgui_time_sink_c_0.set_line_style(2, 'dotdash')
        self.bokehgui_time_sink_c_0.set_line_marker(0, '^')
        self.bokehgui_time_sink_c_0.set_line_marker(3, 'v')
        self.bokehgui_time_sink_c_0.set_line_width(1, 2)

        self.doc.add_root(column(self.plot_lst))
        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_sig_source_x_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.analog_sig_source_x_1, 0), (self.blocks_throttle_1, 0))
        self.connect((self.blocks_throttle_0, 0), (self.bokehgui_time_sink_c_proc_0, 0))
        self.connect((self.blocks_throttle_1, 0), (self.bokehgui_time_sink_c_proc_0, 1))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate)
        self.bokehgui_time_sink_c_0.set_sample_rate(self.samp_rate)

def main(top_block_cls=top_block, options=None):
    serverProc = subprocess.Popen(["bokeh", "serve"])
    time.sleep(1)
    try:
        # Define the document instance
        doc = curdoc()
        session = push_session(doc, session_id="test", url='http://localhost:5006/')
        # Create Top Block instance
        tb = top_block_cls(doc)
        try:
            tb.start()
            session.loop_until_closed()
        except KeyboardInterrupt:
            print "Exiting the simulation. Stopping Bokeh Server"
        finally:
            tb.stop()
	    tb.wait()
    finally:
        serverProc.kill()

if __name__ == '__main__':
    main()
