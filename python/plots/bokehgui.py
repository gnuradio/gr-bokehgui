import imp
import os

imp.load_source("waterfall",
                os.path.join(os.path.dirname(__file__), "waterfall.py"))
