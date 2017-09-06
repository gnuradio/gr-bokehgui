from os.path import dirname, join

from bokeh.core.properties import Color, Float, Int, Override, Seq, Bool
from bokeh.models import Renderer


class WaterfallRenderer(Renderer):
    __implementation__ = join(dirname(__file__), "waterfall.coffee")

    latest = Seq(Float)

    palette = Seq(Color)

    time_length = Int()

    fft_length = Int()

    min_value = Float()

    max_value = Float()

    update = Bool()

    level = Override(default = "glyph")
