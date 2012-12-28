
### Helper imports and decorators
# TODO: separate (nose plugin?)

from unittest import SkipTest

def alltypes(_filter=None):
  def decorate(f):
    def gen():
      for t in core.PixelTypes[1:]:
        if (_filter is None) or _filter(t):
          yield f, t
    gen.__name__ = f.__name__
    return gen
  return decorate


### The actual tests

from gfxprim import core
from gfxprim.core import Context

def test_basic_types_exist():
  assert core.C.PIXEL_RGB888 > 0
  assert core.C.PIXEL_RGBA8888 > 0
  assert core.C.PIXEL_G8 > 0

def test_create_context():
  c = Context.Create(7, 9, core.C.PIXEL_RGB888)
  assert c.w == 7
  assert c.h == 9


@alltypes()
def test_create_context_and_check_sanity(t):
  "Allocate Context by pixeltype and check basic invariants"
  c = Context.Create(13, 15, t.type)
  assert c.w == 13
  assert c.h == 15
  assert c._bit_endian == t.bit_endian
  assert c.bpp == t.size
  assert c._free_pixels

@alltypes()
def test_create_by_number(t):
  "Allocation by pixeltype number"
  c = Context.Create(3, 5, t)

@alltypes()
def test_context_convert_from_RGB888(t):
  "Test conversion from RGB888"
  if 'P' in t.name:
    raise SkipTest("Palette conversion os TODO")
  c = Context.Create(17, 19, core.C.PIXEL_RGB888)
  # both by number and the pixeltype
  c2 = c.Convert(t)
  assert c2.pixel_type == t.type
  c3 = c.Convert(t.type)
  assert c3.pixel_type == t.type

@alltypes()
def test_context_convert_to_RGB888(t):
  "Test conversion to RGB888"
  if 'P' in t.name:
    raise SkipTest("Palette conversion os TODO")
  c = Context.Create(1, 1, t)
  c2 = c.Convert(core.C.PIXEL_RGB888)
  assert c2.pixel_type == core.C.PIXEL_RGB888
