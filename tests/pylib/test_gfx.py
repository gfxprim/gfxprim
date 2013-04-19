"core.Context tests"

from unittest import SkipTest
from testutils import *

from gfxprim.core import Context
from gfxprim import gfx, core


def test_gfx_submodule_loads():
  "gfx is present in a Context"
  c = Context(1, 1, core.C.PIXEL_RGB888)
  assert c.gfx


def test_gfx_submodule_has_C():
  "gfx contains C"
  c = Context(1, 1, core.C.PIXEL_RGB888)
  assert c.gfx.C
  assert gfx.C


# These set the param types of the functions in GFX
gfx_params = {
    'ArcSegment': 'IIIIIFFP',
    'Circle': 'IIIP',
    'Ellipse': 'IIIIP',
    'Fill': 'P',
    'FillCircle': 'IIIP',
    'FillEllipse': 'IIIIP',
    'FillPolygon': ([(0,0),(1,1),(1,0)], 0, {}),
    'FillRect': 'IIIIP',
    'FillRect_AA': 'IIIIP', # Fixpoint, originally 'FFFFP'
    'FillRing': 'IIIIP',
    'FillSymbol': '0IIIIP',
    'FillTetragon': 'IIIIIIIIP',
    'FillTriangle': 'IIIIIIP',
    'HLine': 'IIIP',
    'HLineAA': 'IIIP', # Fixpoint, originally 'FFFP'
    'Line': 'IIIIP',
    'LineAA': 'IIIIP', # Fixpoint, originally 'FFFFP'
    'Polygon': ([(0,0),(1,1),(1,0)], 0, {}),
    'PutPixelAA': 'IIP', # Fixpoint, originally 'FFP'
    'Rect': 'IIIIP',
    'Ring': 'IIIIP',
    'Symbol': '0IIIIP',
    'Tetragon': 'IIIIIIIIP',
    'Triangle': 'IIIIIIP',
    'VLine': 'IIIP',
    'VLineAA': 'IIIP', # Fixpoint, originally 'FFFP'
    }


def test_all_methods_are_known():
  "All methods of gfx submodule have known param types in this test"
  c = Context(1, 1, core.C.PIXEL_RGB888)
  for name in dir(c.gfx):
    if name[0] != '_' and name not in ['C', 'ctx']:
      assert name in gfx_params

def gen_dummy_args(params):
  """
  Generate dummy parameter tuple  according to characters in the given string.

  0 - 0
  S - String ("")
  I - Int (1)
  F - Float (0.5)
  P - Pixel (0)
  """
  args = []
  for t in params:
    if t == '0':
      args.append(0)
    elif t == 'I':
      args.append(1)
    elif t == 'P':
      args.append(0)
    elif t == 'F':
      args.append(0.5)
    elif t == 'S':
      args.append("")
    else:
      assert False
  return tuple(args)

@for_each_case(gfx_params)
def test_method_call(n, params):
  "Calling with dummy parameters:"
  c = ContextRand(10, 10, core.C.PIXEL_RGB888)
  if isinstance(params, str):
    c.gfx.__getattribute__(n)(*gen_dummy_args(params))
  else:
    assert isinstance(params, tuple) and isinstance(params[-1], dict)
    c.gfx.__getattribute__(n)(*params[:-1], **params[-1])

def test_Polygon():
  "Polygon() works"
  c0 = ContextRand(13, 12, core.C.PIXEL_RGB888, seed=42)
  c1 = ContextRand(13, 12, core.C.PIXEL_RGB888, seed=42)
  c2 = ContextRand(13, 12, core.C.PIXEL_RGB888, seed=42)
  assert c1 == c0
  c1.gfx.Polygon([1,2,0,4,7,9,5,4,3,2], 43)
  c2.gfx.Polygon([(1,2),(0,4),(7,9),(5,4),(3,2)], 43)
  assert c1 == c2
  assert c1 != c0

def test_FillPolygon():
  "FillPolygon() works"
  c0 = ContextRand(13, 9, core.C.PIXEL_RGB888, seed=41)
  c1 = ContextRand(13, 9, core.C.PIXEL_RGB888, seed=41)
  c2 = ContextRand(13, 9, core.C.PIXEL_RGB888, seed=41)
  assert c1 == c0
  c1.gfx.FillPolygon([1,2,0,4,7,9,5,4,3,2], 0)
  c2.gfx.FillPolygon([(1,2),(0,4),(7,9),(5,4),(3,2)], 0)
  assert c1 == c2
  assert c1 != c0
