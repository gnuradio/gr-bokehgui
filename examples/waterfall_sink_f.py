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

import time, signal, functools

from gnuradio import analog
from gnuradio import blocks
from gnuradio import gr
from gnuradio.filter import firdes

from bokeh.client import push_session
from bokeh.plotting import curdoc
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
        self.bokehgui_waterfall_sink_f_proc_0 = bokehgui.waterfall_sink_f_proc(1024, firdes.WIN_BLACKMAN_hARRIS, 0, samp_rate/2, "Waterfall Sink", 1)
        self.bokehgui_waterfall_sink_f_0 = bokehgui.waterfall_sink_f(self.doc, self.plot_lst, self.bokehgui_waterfall_sink_f_proc_0)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_float*1, samp_rate,True)
#        self.blocks_throttle_1 = blocks.throttle(gr.sizeof_float*1, samp_rate,True)
        self.analog_sig_source_x_0 = analog.sig_source_f(samp_rate, analog.GR_COS_WAVE, 5000, 3, 0)
#        self.analog_sig_source_x_1 = analog.sig_source_f(samp_rate, analog.GR_COS_WAVE, 1000, 1, 0)
        self.analog_noise_source_x_0 = analog.noise_source_f(analog.GR_GAUSSIAN, 0.001, 0)
#        self.analog_noise_source_x_1 = analog.noise_source_f(analog.GR_GAUSSIAN, 0.01, 0)
        self.blocks_add_xx_0 = blocks.add_vff(1)
#        self.blocks_add_xx_1 = blocks.add_vff(1)

        ##################################################
        # Customizing the plot
        ##################################################
        self.bokehgui_waterfall_sink_f_0.initialize(legend_list = ['Signal (5000 Hz)',
 #                                                                  'Signal (1000 Hz)',
                                                                  ],
                                                    update_time = 100
                                                    )

#        self.bokehgui_freq_sink_f_0.set_x_label('Frequency (Hz)')
#        self.bokehgui_freq_sink_f_0.set_y_label('Relative Gain (dB)')
#        self.bokehgui_freq_sink_f_0.set_y_axis([-150, 10])
#        self.bokehgui_freq_sink_f_0.set_line_color(0, 'blue')
#        self.bokehgui_freq_sink_f_0.set_line_color(1, 'red')
#        self.bokehgui_freq_sink_f_0.set_line_style(1, 'dashed')
#        self.bokehgui_freq_sink_f_0.set_line_width(1, 1)
#        self.bokehgui_freq_sink_f_0.enable_max_hold()
        self.bokehgui_waterfall_sink_f_0.set_layout(1,1,1,1)

        self.doc.add_root(bokehgui.BokehLayout.create_layout(self.plot_lst))
        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_sig_source_x_0, 0), (self.blocks_add_xx_0, 0))
#        self.connect((self.analog_sig_source_x_1, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.analog_noise_source_x_0, 0), (self.blocks_add_xx_0, 1))
#        self.connect((self.analog_noise_source_x_1, 0), (self.blocks_add_xx_1, 1))
        self.connect((self.blocks_add_xx_0, 0), (self.blocks_throttle_0, 0))
#        self.connect((self.blocks_add_xx_1, 0), (self.blocks_throttle_0, 0))
        self.connect((self.blocks_throttle_0, 0), (self.bokehgui_waterfall_sink_f_proc_0, 0))
#        self.connect((self.blocks_throttle_1, 0), (self.bokehgui_freq_sink_f_proc_0, 1))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate)
        self.bokehgui_waterfall_sink_f_0.set_frequency_range([0, self.samp_rate/2])

def main(top_block_cls=top_block, options=None):
    serverProc = bokehgui.utils.create_server()
    def killProc(signum, frame, tb):
        tb.stop()
        tb.wait()
        serverProc.terminate()
        serverProc.kill()
    time.sleep(1)
    try:
        # Define the document instance
        doc = curdoc()
        session = push_session(doc, session_id="test", url = "http://localhost:5006/bokehgui")
        # Create Top Block instance
        tb = top_block_cls(doc)
        try:
            tb.start()
            signal.signal(signal.SIGTERM, functools.partial(killProc, tb=tb))
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
