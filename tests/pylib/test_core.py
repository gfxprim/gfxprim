### Helper imports and decorators
# TODO: separate (nose plugin?)

from unittest import SkipTest
from gfxprim import core
from gfxprim.core import Context
from random import Random


def alltypes(_filter=None):
  def decorate(f):
    def gen():
      for t in core.PixelTypes[1:]:
        if (_filter is None) or _filter(t):
          yield f, t
    gen.__name__ = f.__name__
    return gen
  return decorate

def ContextRand(w, h, t, seed=None):
  "Return new Context(w, h, t) filled with RandomizeContext(c, seed)"
  c = Context(w, h, t)
  RandomizeContext(c, seed)
  return c

def RandomizeContext(c, seed=None):
  if seed is None:
    seed = c.w + (2 ** c.h) * c.pixel_type
  r = Random(seed)
  for x in range(c.w):
    for y in range(c.h):
      c.PutPixel(x, y, r.randint(0, (1 << c.bpp) - 1))

### The actual tests

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
    raise SkipTest("Palette conversion are TODO")
  c = ContextRand(17, 19, core.C.PIXEL_RGB888)
  # both by number and the pixeltype
  c2 = c.Convert(t)
  assert c2.pixel_type == t.type
  c3 = c.Convert(t.type)
  assert c3.pixel_type == t.type

@alltypes()
def test_convert_to_RGB888(t):
  "Conversion to RGB888"
  if 'P' in t.name:
    raise SkipTest("Palette conversion are TODO")
  c = ContextRand(11, 12, t)
  c2 = c.Convert(core.C.PIXEL_RGB888)
  assert c2.pixel_type == core.C.PIXEL_RGB888

@alltypes()
def test_equality(t):
  "Equality"
  c1a = ContextRand(2, 11, t, seed=123)
  c1b = ContextRand(2, 11, t, seed=123)
  assert c1a == c1a
  assert c1a == c1b
  c2 = ContextRand(2, 11, t, seed=456)
  assert c2 == c2
  assert c1a != c2
  assert c2 != c1a

@alltypes()
def test_get_put_pixel(t):
  "Get/Put pixel consistent"
  def f(x,y):
    return (x + 3 ** y) % (1 << t.size)
  c = ContextRand(45, 37, t)
  for x in range(c.w):
    for y in range(c.h):
      c.PutPixel(x, y, f(x, y))
  for x in range(c.w):
    for y in range(c.h):
      assert c.GetPixel(x, y) == f(x, y)

@alltypes()
def test_str_repr(t):
  c = Context(42, 43, t)
  assert "42x43" in str(c)
  assert "42x43" in repr(c)
  assert t.name in str(c)
  assert t.name in repr(c)

@alltypes()
def test_blit_vs_convert_to_RGB888(t):
  if 'P' in t.name:
    raise SkipTest("Palette conversions are TODO")
  c = ContextRand(42, 43, t)
  c2a = Context(c.w, c.h, core.C.PIXEL_RGB888)
  c.Blit(0, 0, c2a, 0, 0, w=c.w, h=c.h)
  c2b = c.Convert(core.C.PIXEL_RGB888)
  assert c2a == c2b

#'Blit',
#   'Copy',
#      'RGBAToPixel',
#       'RGBToPixel',
#        'Resize',
#           'SubContext',

