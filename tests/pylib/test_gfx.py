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
    'ArcSegment': 'CCCCCFFP',
    'Circle': 'CCCP',
    'Ellipse': 'CCCCP',
    'Fill': 'P',
    'FillCircle': 'CCCP',
    'FillEllipse': 'CCCCP',
    'FillPolygon': None,
    'FillRect': 'CCCCP',
    'FillRect_AA': 'FFFFP',
    'FillRing': 'CCCCP',
    'FillSymbol': '0CCCCP',
    'FillTetragon': 'CCCCCCCCP',
    'FillTriangle': 'CCCCCCP',
    'HLine': 'CCCP',
    'HLineAA': 'FFFP',
    'Line': 'CCCCP',
    'LineAA': 'FFFFP',
    'Polygon': None,
    'PutPixelAA': 'FFP',
    'Rect': 'CCCCP',
    'Ring': 'CCCCP',
    'Symbol': '0CCCCP',
    'Tetragon': 'CCCCCCCCP',
    'Triangle': 'CCCCCCP',
    'VLine': 'CCCP',
    'VLineAA': 'FFFP',
    }


def test_all_methods_are_known():
  "All methods of gfx submodule have known param types in this test"
  c = Context(1, 1, 1)
  for name in dir(c.gfx):
    if name[0] != '_' and name not in ['C', 'ctx']:
      assert name in gfx_params


@for_each_case(gfx_params, _filter=(lambda(n, params): params is not None))
def test_method_callable(n, params):
  "Call with dummy parameters"
  c = Context(10, 10, 1)
  args = []
  for t in params:
    if t == '0':
      args.append(0)
    elif t == 'C':
      args.append(1)
    elif t == 'P':
      args.append(0)
    elif t == 'F':
      args.append(0.5)
    else:
      assert False
  print args
  c.gfx.__getattribute__(n)(*tuple(args))

