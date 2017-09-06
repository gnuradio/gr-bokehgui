import * as p from "core/properties"
import {Renderer, RendererView} from "models/renderers/renderer"
import {LinearColorMapper} from "models/mappers/linear_color_mapper"

export class WaterfallRendererView extends RendererView
  initialize: (options) ->
    super(options)

    @model.update = false

    N = 11
    @model.tile_height = @model.time_length / 10
    [w, h] = [@model.fft_length, @model.tile_height]

    @image = []
    @canvas = []
    for i in [0...N]
      canvas = document.createElement('canvas')
      [canvas.width, canvas.height] = [w, h]
      @canvas.push(canvas)
      @image.push(new Uint32Array(w*h))

    @y = new Array(N)
    for i in [0...N]
      @y[i] = @model.time_length - @model.tile_height*(i-1)

    [@row, @tile] = [0, 0]
    @cmap = new LinearColorMapper({'palette': @model.palette, low: @model.min_value, high: @model.max_value})
    @xscale = @plot_view.frame.xscales['default']
    @yscale = @plot_view.frame.yscales['default']
    @max_freq = @plot_view.frame.x_range.end
    @min_freq = @plot_view.frame.x_range.start

    @connect(@model.change, @request_render)

  render: () ->
    @cmap = new LinearColorMapper({'palette': @model.palette, low: @model.min_value, high: @model.max_value})
    ctx = @plot_view.canvas_view.ctx

    for i in [0...@y.length]
      @y[i] -= 1

    @row -= 1
    if @row < 0
      @row = @model.tile_height - 1
      @tile -= 1
      if @tile < 0
        @tile = @y.length - 1
      @y[@tile] = @model.time_length + @model.tile_height

    buf32 = new Uint32Array(@cmap.v_map_screen(@model.latest))
    for i in [0...@model.fft_length]
      @image[@tile][i+@model.fft_length*(@row)] = buf32[i]

    sx = @plot_view.canvas.vx_to_sx(@xscale.map_to_target(@min_freq))
    sy = @plot_view.canvas.v_vy_to_sy(@yscale.v_map_to_target(@y))
    sw = Math.ceil(@xscale.map_to_target(@max_freq) - @xscale.map_to_target(@min_freq))
    sh = Math.ceil(@yscale.map_to_target(@model.tile_height) - @yscale.map_to_target(0))
    ctx.save()

    smoothing = ctx.getImageSmoothingEnabled()
    ctx.setImageSmoothingEnabled(false)

    for i in [0...sy.length]
      if i == @tile
        cctx = @canvas[i].getContext('2d')
        image = cctx.getImageData(0, 0, @model.fft_length, @model.tile_height)
        image.data.set(new Uint8Array(@image[i].buffer))
        cctx.putImageData(image, 0, 0)
      ctx.drawImage(@canvas[i], sx, sy[i], sw, sh)

    ctx.setImageSmoothingEnabled(smoothing)

    ctx.restore()

export class WaterfallRenderer extends Renderer
  type: 'WaterfallRenderer'
  default_view: WaterfallRendererView
  @define {
    latest:      [ p.Any ]
    palette:     [ p.Any ]
    time_length: [ p.Int ]
    fft_length: [ p.Int ]
    min_value: [ p.Any ]
    max_value: [ p.Any ]
    update: [ p.Any ]
  }
  @override { level: "glyph" }

