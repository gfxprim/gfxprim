"core.Context tests"

from unittest import SkipTest
from testutils import *

from gfxprim.core import Context
from gfxprim import gfx


def test_gfx_submodule_loads():
  "gfx is present in a Context"
  c = Context(1, 1, 1)
  assert c.gfx


def test_gfx_submodule_has_C():
  "gfx contains C"
  c = Context(1, 1, 1)
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
    'FillRect_AA': 'FFFFP',
    'FillRing': 'IIIIP',
    'FillSymbol': '0IIIIP',
    'FillTetragon': 'IIIIIIIIP',
    'FillTriangle': 'IIIIIIP',
    'HLine': 'IIIP',
    'HLineAA': 'FFFP',
    'Line': 'IIIIP',
    'LineAA': 'FFFFP',
    'Polygon': ([(0,0),(1,1),(1,0)], 0, {}),
    'PutPixelAA': 'FFP',
    'Rect': 'IIIIP',
    'Ring': 'IIIIP',
    'Symbol': '0IIIIP',
    'Tetragon': 'IIIIIIIIP',
    'Triangle': 'IIIIIIP',
    'VLine': 'IIIP',
    'VLineAA': 'FFFP',
    }


def test_all_methods_are_known():
  "All methods of gfx submodule have known param types in this test"
  c = Context(1, 1, 1)
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
def test_method_callable(n, params):
  "Call with dummy parameters"
  c = Context(10, 10, 1)
  if isinstance(params, str):
    c.gfx.__getattribute__(n)(*gen_dummy_args(params))
  else:
    assert isinstance(params, tuple) and isinstance(params[-1], dict)
    c.gfx.__getattribute__(n)(*params[:-1], **params[-1])

