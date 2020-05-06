#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Time Sink Float values example
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
from bokeh.client import push_session
from bokeh.plotting import curdoc
from gnuradio import analog, blocks, gr
from gnuradio.filter import firdes


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
        self.bokehgui_waterfall_sink_c_proc_0 = bokehgui.waterfall_sink_c_proc(
            1024, firdes.WIN_BLACKMAN_hARRIS, 0, samp_rate / 2,
            "Waterfall Sink")
        self.bokehgui_waterfall_sink_c_0 = bokehgui.waterfall_sink_c(self.doc,
                                                                     self.plot_lst,
                                                                     self.bokehgui_waterfall_sink_c_proc_0)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_gr_complex * 1,
                                                 samp_rate, True)
        self.analog_sig_source_x_0 = analog.sig_source_c(samp_rate,
                                                         analog.GR_COS_WAVE,
                                                         000, 3, 0)
        self.analog_noise_source_x_0 = analog.noise_source_c(
            analog.GR_GAUSSIAN, 0.01, 0)
        self.blocks_add_xx_0 = blocks.add_vcc(1)

        ##################################################
        # Customizing the plot
        ##################################################
        self.bokehgui_waterfall_sink_c_0.initialize(
            legend_list = ['Signal (5000 Hz)', ], update_time = 100)

        self.bokehgui_waterfall_sink_c_0.set_x_label('Frequency (Hz)')
        self.bokehgui_waterfall_sink_c_0.set_y_label('Time')
        self.bokehgui_waterfall_sink_c_0.set_layout(1, 1, 1, 1)

        self.doc.add_root(bokehgui.bokeh_layout.create_layout(self.plot_lst))
        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_sig_source_x_0, 0),
                     (self.blocks_add_xx_0, 0))
        self.connect((self.analog_noise_source_x_0, 0),
                     (self.blocks_add_xx_0, 1))
        self.connect((self.blocks_add_xx_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.blocks_throttle_0, 0),
                     (self.bokehgui_waterfall_sink_c_proc_0, 0))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate)
        self.bokehgui_waterfall_sink_c_0.set_frequency_range(
                [0, self.samp_rate / 2])


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
