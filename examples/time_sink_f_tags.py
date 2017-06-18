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

from gnuradio import analog
from gnuradio import blocks
from gnuradio import gr
from gnuradio.filter import firdes
import bokehgui
from bokeh.client import push_session
from bokeh.plotting import curdoc
from tornado.ioloop import IOLoop
from bokeh.application import Application
from bokeh.server.server import Server
import pmt

class top_block(gr.top_block):
    def __init__(self, doc):
        gr.top_block.__init__(self, "Top Block")
        self.doc = doc

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 32000

        ##################################################
        # Blocks
        ##################################################
        self.bokehgui_time_sink_f_proc_0 = bokehgui.time_sink_f_proc(500, samp_rate, 'TimeSink', 2)
        self.bokehgui_time_sink_f_0 = bokehgui.time_sink_f(self.doc, self.bokehgui_time_sink_f_proc_0, 500, samp_rate, 'TImeSink', 2)
        self.bokehgui_time_sink_f_0.set_trigger_mode(bokehgui.TRIG_MODE_FREE,bokehgui.TRIG_SLOPE_POS,0,0,0,"")
        self.bokehgui_time_sink_f_0.initialize(legend_list=['data0','data1'])

        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_float*1, samp_rate,True)
        self.blocks_throttle_1 = blocks.throttle(gr.sizeof_float*1, samp_rate,True)
        self.tagged_1 = blocks.tags_strobe(gr.sizeof_float*1, pmt.intern("TEST"), 100, pmt.intern("strobe1"))
        self.tagged_0 = blocks.tags_strobe(gr.sizeof_float*1, pmt.intern("TEST"), 100, pmt.intern("strobe2"))
        self.adder = blocks.add_vff(1)
        self.adder_1 = blocks.add_vff(1)
        self.analog_sig_source_x_0 = analog.sig_source_f(samp_rate, analog.GR_COS_WAVE, 1000, 1, 0)
        self.analog_sig_source_x_1 = analog.sig_source_f(samp_rate, analog.GR_COS_WAVE, 100, 1, 0)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_sig_source_x_0, 0), (self.adder, 0))
        self.connect((self.tagged_0, 0), (self.adder, 1))
        self.connect((self.analog_sig_source_x_1, 0), (self.adder_1, 0))
        self.connect((self.tagged_1, 0), (self.adder_1, 1))
        self.connect((self.adder, 0), (self.blocks_throttle_0, 0))
        self.connect((self.adder_1, 0), (self.blocks_throttle_1, 0))
        self.connect((self.blocks_throttle_0, 0), (self.bokehgui_time_sink_f_proc_0, 0))
        self.connect((self.blocks_throttle_1, 0), (self.bokehgui_time_sink_f_proc_0, 1))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate)
        self.bokehgui_time_sink_f_0.set_sample_rate(self.samp_rate)

def main(top_block_cls=top_block, options=None):
    # Define tornado loop
    loop = IOLoop()

    # Define a blank application
    app = Application()
    # Starting server at port 5006
    srv = Server({'/':app},io_loop=loop)
    # Start server process
    srv.start()
   # Define the document instance
    doc = curdoc()
    session = push_session(doc, session_id="test", io_loop=loop, url='http://localhost:5006/')

    # Create Top Block instance
    tb = top_block_cls(doc)

    # Start simulations as soon as the server starts
    loop.add_callback(tb.start)
    loop.add_callback(srv.show,'/?bokeh-session-id='+str(session.id))
    try:
        loop.start()
    except KeyboardInterrupt:
        print "Exiting the simulation. Stopping Bokeh Server"
    except Exception as e:
        print str(e)
    finally:
        # Stop the simulations when there is a key interrupt
        tb.stop()
        tb.wait()


if __name__ == '__main__':
    main()

