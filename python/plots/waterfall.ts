import {Renderer, RendererView} from "models/renderers/renderer"
import {LinearColorMapper} from "models/mappers/linear_color_mapper"
import {Scale} from "models/scales/scale"
import {Color} from "core/types"
import {canvas} from "core/dom"
import * as p from "core/properties"

export class WaterfallRendererView extends RendererView {
  model: WaterfallRenderer

  private canvas: HTMLCanvasElement[]
  private image: Uint32Array[]
  private y: number[]
  private row: number
  private tile: number
  private cmap: LinearColorMapper
  private xscale: Scale
  private yscale: Scale
  private min_freq: number
  private max_freq: number
  private tile_height: number

  initialize(): void {
    super.initialize()

    this.model.update = false

    const N = 11
    this.tile_height = this.model.time_length / 10
    const [w, h] = [this.model.fft_length, this.tile_height]

    this.canvas = []
    this.image = []
    for (let i = 0; i < N; i++) {
      this.canvas.push(canvas({width: w, height: h}))
      this.image.push(new Uint32Array(w*h))
    }

    this.y = new Array(N)
    for (let i = 0; i < N; i++)
      this.y[i] = this.model.time_length + this.tile_height*(i-1)

    this.row = 0
    this.tile = 0
    this.cmap = new LinearColorMapper({palette: this.model.palette, low: this.model.min_value, high: this.model.max_value})
    this.xscale = this.plot_view.frame.xscales.default
    this.yscale = this.plot_view.frame.yscales.default
    this.min_freq = this.plot_view.frame.x_range.start
    this.max_freq = this.plot_view.frame.x_range.end
  }

  connect_signals(): void {
    super.connect_signals()
    this.connect(this.model.change, this.request_render)
  }

  protected _render(): void {
    const ctx = this.layer.ctx
    ctx.save()

    const smoothing = ctx.getImageSmoothingEnabled()
    ctx.setImageSmoothingEnabled(false)

    this._update_tiles()

    const sx = this.xscale.compute(this.min_freq)
    const sy = this.yscale.v_compute(this.y)
    const sw = Math.ceil(this.xscale.compute(this.max_freq) - this.xscale.compute(this.min_freq))
    const sh = Math.ceil(this.yscale.compute(this.tile_height) - this.yscale.compute(0))

    for (let i = 0; i < sy.length; i++)
      ctx.drawImage(this.canvas[i], sx, sy[i], sw, sh)

    ctx.setImageSmoothingEnabled(smoothing)

    ctx.restore()
  }

  _update_tiles(): void {
    // shift all tiles down by one
    for (let i = 0; i < this.y.length; i++)
      this.y[i] -= 1

    // if we've updated the last row in the current tile, move to the next tile
    // in the buffer (rotating the buffer if necessary)
    this.row -= 1
    if (this.row < 0) {
      this.row = this.tile_height - 1
      this.tile -= 1
      if (this.tile < 0)
        this.tile = this.y.length - 1
      this.y[this.tile] = this.model.time_length + this.tile_height
    }

    // apply the lastest column to the current tile image
    const buf32 = new Uint32Array(this.cmap.rgba_mapper.v_compute(this.model.latest).buffer)
    for (let i = 0; i < this.model.fft_length; i++)
      this.image[this.tile][i+this.model.fft_length*this.row] = buf32[i]

    // update the tiles canvas with the image data
    const cctx = this.canvas[this.tile].getContext('2d')!
    const image = cctx.getImageData(0, 0, this.model.fft_length, this.tile_height)
    image.data.set(new Uint8Array(this.image[this.tile].buffer))
    cctx.putImageData(image, 0, 0)
  }
}

export namespace WaterfallRenderer {
  export type Attrs = p.AttrsOf<Props>

  export type Props = Renderer.Props & {
    latest:      p.Property<number[]>
    palette:     p.Property<Color[]>
    time_length: p.Property<number>
    fft_length:  p.Property<number>
    min_value:   p.Property<number>
    max_value:   p.Property<number>
    update:      p.Property<boolean>
  }
}

export interface WaterfallRenderer extends WaterfallRenderer.Attrs {}

export class WaterfallRenderer extends Renderer {
  properties: WaterfallRenderer.Props

  constructor(attrs?: Partial<WaterfallRenderer.Attrs>) {
    super(attrs)
  }

  static initClass(): void {
    this.prototype.default_view = WaterfallRendererView

    this.define<WaterfallRenderer.Props>({
      latest:      [ p.Any ],
      palette:     [ p.Any ],
      time_length: [ p.Int ],
      fft_length:  [ p.Int ],
      min_value:   [ p.Any ],
      max_value:   [ p.Any ],
      update:      [ p.Any ],
    })

    this.override({
      level: "glyph",
    })
  }
}
WaterfallRenderer.initClass()
