"""
Module extending the Context class with .gfx submodule and its drawing functions.

Use as in "import gfxprim.gfx; context_foo.gfx.Line(...)"
"""

# Import the SWIG wrapper
from . import c_gfx

# Constants module
from . import C

def _init(module):
  "Extend Context with gfx submodule"

  from ..utils import extend, add_swig_getmethod, add_swig_setmethod
  from ..core import Context as _context

  # New Context submodule
  class GfxSubmodule(object):
    def __init__(self, ctx):
      self.ctx = ctx
      self.C = C

  _context._submodules['gfx'] = GfxSubmodule

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
      'ArcSegment', 'Circle', 'Ellipse', 'Fill', 'FillCircle', 'FillEllipse',
      'FillRect', 'FillRect_AA', 'FillRing',
      'FillTetragon', 'FillTriangle', 'HLine', 'HLineAA', 'Line', 'LineAA',
      'PutPixelAA', 'Rect', 'Ring', 'Tetragon',
      'Triangle', 'VLine', 'VLineAA']:
    extend_submodule(GfxSubmodule, name, c_gfx.__getattribute__('GP_' + name))

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
  def Polygon(self, points, pixel):
    """
    Polygon(context, coordinates, pixel)
    
    Draw a polygon with color `pixel`.
    `coordinates` is either an iterable of `int` coordinates `(x0, y0, x1, y1, ...)`
    or an iterable of tuples `[(x0, y0), (x1, y1), ...]`.
    """
    c_gfx.GP_Polygon_wrap(self.ctx, flatten_coords(points), pixel)

  @extend(GfxSubmodule)
  def FillPolygon(self, points, pixel):
    """
    FillPolygon(context, coordinates, pixel)
    
    Draw a filled polygon with color `pixel`.
    `coordinates` is either an iterable of `int` coordinates `(x0, y0, x1, y1, ...)`
    or an iterable of tuples `[(x0, y0), (x1, y1), ...]`.
    """
    c_gfx.GP_FillPolygon_wrap(self.ctx, flatten_coords(points), pixel)

_init(locals())
del _init
