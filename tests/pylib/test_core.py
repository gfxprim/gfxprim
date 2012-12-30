"core.Context"

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

@alltypes()
def test_create_by_pixeltype(t):
  "Allocate Context by pixeltype"
  c = Context(13, 15, t.type)

@alltypes()
def test_create_by_number(t):
  "Allocation by pixeltype number"
  c = Context(3, 5, t)

@alltypes()
def test_check_attributes(t):
  "Context attributes"
  c = Context(13, 15, t.type)
  assert c.w == 13
  assert c.h == 15
  assert c._bit_endian == t.bit_endian
  assert c.bpp == t.size
  assert c._free_pixels

@alltypes()
def test_context_convert_from_RGB888(t):
  "Conversion from RGB888"
  if 'P' in t.name:
    raise SkipTest("Palette conversion os TODO")
  c = Context(17, 19, core.C.PIXEL_RGB888)
  # both by number and the pixeltype
  c2 = c.Convert(t)
  assert c2.pixel_type == t.type
  c3 = c.Convert(t.type)
  assert c3.pixel_type == t.type

@alltypes()
def test_convert_to_RGB888(t):
  "Conversion to RGB888"
  if 'P' in t.name:
    raise SkipTest("Palette conversion os TODO")
  c = Context(1, 1, t)
  c2 = c.Convert(core.C.PIXEL_RGB888)
  assert c2.pixel_type == core.C.PIXEL_RGB888

@alltypes()
def test_equality_same_type(t):
  "Basics of equality"
  c1 = Context(2, 1, t)
  assert c1 == c1
  c2 = Context(1, 2, t)
  assert c2 == c2
  assert c1 != c2
  assert c2 != c1

@alltypes()
def test_equality_data(t):
  "Equality of data"
  c1 = Context(1, 1, t)
  c1.PutPixel(0, 0, 1)
  c2 = Context(1, 1, t)
  c2.PutPixel(0, 0, 1)
  assert c1 == c2
  c2.PutPixel(0, 0, 0)
  assert c1 != c2

#@alltypes()
#def test_equality_data(t):
