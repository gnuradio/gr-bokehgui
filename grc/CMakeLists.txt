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

install(FILES
    bokehgui_time_sink_x.xml 
    bokehgui_freq_sink_x.xml 
    bokehgui_label.xml
    bokehgui_slider.xml
    bokehgui_range_slider.xml
    bokehgui_checkbox.xml
    bokehgui_radio_button.xml
    bokehgui_textbox.xml     
    bokehgui_waterfall_sink_x.xml
    DESTINATION share/gnuradio/grc/blocks)
