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
        self.bokehgui_time_sink_f_proc_0 = bokehgui.time_sink_f_proc(750, samp_rate, 'TimeSink', 2)
        self.bokehgui_time_sink_f_0 = bokehgui.time_sink_f(self.doc, self.bokehgui_time_sink_f_proc_0)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_float*1, samp_rate,True)
        self.blocks_throttle_1 = blocks.throttle(gr.sizeof_float*1, samp_rate,True)
        self.analog_sig_source_x_0 = analog.sig_source_f(samp_rate, analog.GR_COS_WAVE, 500, 3, 0)
        self.analog_sig_source_x_1 = analog.sig_source_f(samp_rate, analog.GR_COS_WAVE, 100, 1, 0)


        ##################################################
        # Customizing the plot
        ##################################################
        self.bokehgui_time_sink_f_0.initialize(legend_list = ['data0',
                                                              'data1',
                                                             ],
                                               update_time = 100
                                               )

        self.bokehgui_time_sink_f_0.set_x_label('Time (s)')
        self.bokehgui_time_sink_f_0.set_y_label('Value')
        self.bokehgui_time_sink_f_0.set_line_color(0, 'black')
        self.bokehgui_time_sink_f_0.set_line_color(1, 'red')
        self.bokehgui_time_sink_f_0.set_line_style(1, 'dashed')
        self.bokehgui_time_sink_f_0.set_line_marker(0, '^')
        self.bokehgui_time_sink_f_0.set_line_width(1, 2)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_sig_source_x_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.analog_sig_source_x_1, 0), (self.blocks_throttle_1, 0))
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
    srv = Server({'/':app},io_loop=loop, allow_websocket_origin=['*'])
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

