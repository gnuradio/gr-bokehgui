# gr-bokehgui: Web based display for GNU Radio
------------------------------

## Overview
<img align="right" src="https://wiki.gnuradio.org/images/2/2f/Gnuradio_logo_icon.png?054f8" width="12%" />

The module provides various sinks and widgets to allow interaction with the live GNU Radio applications remotely over the network. The module uses [Bokeh](https://docs.bokeh.org/en/1.4.0/)'s client API and streaming features. Using widgets and plots from Bokeh, the module enables the GUI for GNU Radio that renders in web browser. Just like `gr-qtgui`, it is fully integrated with GRC to allow easy use.

The module was irst developed as a part of Google Summer of Code 2017 by Kartik Patel.


## Dependency
1. GNU Radio 3.8.3
2. [Bokeh library](https://docs.bokeh.org/en/2.0.2/)
   (Tested on v1.4.0 and 2.0.2) (later versions cause the waterfall display to crash)

## Installation

### Using source code
1. Make sure that you have satisfied all dependency listed above.
2. Clone the latest code
```
$ git clone https://github.com/gnuradio/gr-bokehgui.git
```
3. Build with CMake:
```
$ cd gr-bokehgui/
$ git checkout tags/compatbokeh2.0
$ mkdir build
$ cd build/
$ cmake ../
$ make
$ sudo make install
```

## Working with the module
The complete tutorial for the module is available [here](http://kartikpatel.in/GSoC2017/tutorial/). The tutorial covers setting up guide, using sinks and widgets in GRC and guide to the placement of the elements.

## Quick Glance
For the following flowgraph:
![tutorial.grc - flowgraph](http://kartikpatel.in/GSoC2017/images/tutorial/tutorial.grc.png)<br>

The output should be as follows:
![tutorial.grc - output](http://kartikpatel.in/GSoC2017/images/tutorial/tutorial.png)<br>

The following Youtube video provides quick glance over the module and basic procedures.
[![Youtube - demo](http://img.youtube.com/vi/EyNOE9icNVc/0.jpg)](https://www.youtube.com/watch?v=EyNOE9icNVc)

The `test_bokehgui.grc` example flowgraph contains all the plots and most widgets to check that everything works correctly.

## Bugs reporting:
Kindly report any bugs or issues [on Github](https://github.com/gnuradio/gr-bokehgui/issues/).

## Contributing to the project
If you want to contribute to the module, feel free to add the pull request. Please read the contribution guidelines of GNU Radio [here](https://wiki.gnuradio.org/index.php/Development).

## Contact
For queries or feedbacks, drop a mail to [discuss-gnuradio](mailto:discuss-gnuradio@gnu.org).

## License
The project is licensed under GPLv3. See LICENSE for terms.
