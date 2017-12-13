"""
Module extending the Pixmap class with .gfx submodule and its drawing functions.

Use as in "import gfxprim.gfx; pixmap_foo.gfx.Line(...)"
"""

# Import the SWIG wrapper
from . import c_gfx

# Constants module
from . import C

def _init(module):
  "Extend Pixmap with gfx submodule"

  from ..utils import extend, add_swig_getmethod, add_swig_setmethod
  from ..core import Pixmap as _pixmap

  # New Pixmap submodule
  class GfxSubmodule(object):
    def __init__(self, ctx):
      self.ctx = ctx
      self.C = C

  _pixmap._submodules['gfx'] = GfxSubmodule

  # Imports from the SWIG module
  from ..utils import import_members, extend_submodule
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  const_regexes = [
    '^GP_[A-Z0-9_]*$',
    ]
  import_members(c_gfx, C, include=const_regexes, sub=strip_GP)

  for name in [
      'arc_segment', 'circle', 'ellipse', 'fill_circle', 'fill_ellipse',
      'fill_rect', 'fill_ring',
      'fill_tetragon', 'fill_triangle', 'hline', 'hline_aa', 'line', 'line_aa',
      'putpixel_aa', 'rect', 'ring', 'tetragon',
      'triangle', 'vline', 'vline_aa']:
    extend_submodule(GfxSubmodule, name, c_gfx.__getattribute__('gp_' + name))

  @extend(GfxSubmodule)
  def fill(self, color):
      self.ctx.fill(color)

  def flatten_coords(points):
    "Helper for Polygon and FillPolygon coordinates"
    l = []
    for p in points:
      if hasattr(p, '__iter__'):
        l.extend(p)
      else:
        l.append(p)
    for i in l:
      assert isinstance(i, int)
    return tuple(l)

  @extend(GfxSubmodule)
  def polygon(self, points, pixel):
    """
    polygon(pixmap, coordinates, pixel)

    Draw a polygon with color `pixel`.
    `coordinates` is either an iterable of `int` coordinates `(x0, y0, x1, y1, ...)`
    or an iterable of tuples `[(x0, y0), (x1, y1), ...]`.
    """
    c_gfx.gp_polygon_wrap(self.ctx, flatten_coords(points), pixel)

  @extend(GfxSubmodule)
  def fill_polygon(self, points, pixel):
    """
    fill_polygon(pixmap, coordinates, pixel)

    Draw a filled polygon with color `pixel`.
    `coordinates` is either an iterable of `int` coordinates `(x0, y0, x1, y1, ...)`
    or an iterable of tuples `[(x0, y0), (x1, y1), ...]`.
    """
    c_gfx.gp_fill_polygon_wrap(self.ctx, flatten_coords(points), pixel)

_init(locals())
del _init
