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
      'FillPolygon', 'FillRect', 'FillRect_AA', 'FillRing', 'FillSymbol',
      'FillTetragon', 'FillTriangle', 'HLine', 'HLineAA', 'Line', 'LineAA',
      'Polygon', 'PutPixelAA', 'Rect', 'Ring', 'Symbol', 'Tetragon',
      'Triangle', 'VLine', 'VLineAA']:
    extend_submodule(GfxSubmodule, name, c_gfx.__getattribute__('GP_' + name))


_init(locals())
del _init
