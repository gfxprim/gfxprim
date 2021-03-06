"core.Pixmap tests"

from unittest import SkipTest
from testutils import *

from gfxprim import core
from gfxprim.core import Pixmap

def test_basic_types_exist():
  "There are the basic pixel types"
  assert core.C.PIXEL_RGB888 > 0
  assert core.C.PIXEL_RGBA8888 > 0
  assert core.C.PIXEL_G8 > 0

@alltypes()
def test_create_by_pixeltype(t):
  "Allocate Pixmap by pixeltype"
  c = Pixmap(13, 15, t.type)

@alltypes()
def test_create_by_number(t):
  "Allocation by pixeltype number"
  c = Pixmap(3, 5, t)

@alltypes()
def test_check_attributes(t):
  "Pixmap attributes"
  c = Pixmap(13, 15, t.type)
  assert c.w == 13
  assert c.h == 15
  assert c._bit_endian == t.bit_endian
  assert c.bpp == t.size
  assert c._free_pixels

@alltypes()
def test_convert_from_RGB888(t):
  "Conversion from RGB888"
  if 'P' in t.name:
    raise SkipTest("Palette conversion are TODO")
  c = PixmapRand(17, 19, core.C.PIXEL_RGB888)
  # both by number and the pixeltype
  c2 = c.convert(t)
  assert c2.pixel_type == t.type
  c3 = c.convert(t.type)
  assert c3.pixel_type == t.type

@alltypes()
def test_convert_to_RGB888(t):
  "Conversion to RGB888"
  if 'P' in t.name:
    raise SkipTest("Palette conversion are TODO")
  c = PixmapRand(11, 12, t)
  c2 = c.convert(core.C.PIXEL_RGB888)
  assert c2.pixel_type == core.C.PIXEL_RGB888

@alltypes()
def test_equality(t):
  "Equality"
  c1a = PixmapRand(2, 11, t, seed=123)
  c1b = PixmapRand(2, 11, t, seed=123)
  assert c1a == c1a
  assert c1a == c1b
  c2 = PixmapRand(2, 11, t, seed=456)
  assert c2 == c2
  assert c1a != c2
  assert c2 != c1a

@alltypes()
def test_get_put_pixel(t):
  "Get/Put pixel consistent"
  def f(x,y):
    return (x + 3 ** y) % (1 << t.size)
  c = PixmapRand(45, 37, t)
  for x in range(c.w):
    for y in range(c.h):
      c.putpixel(x, y, f(x, y))
  for x in range(c.w):
    for y in range(c.h):
      assert c.getpixel(x, y) == f(x, y)

@alltypes()
def test_str_repr(t):
  "Pixmap __str__ and __repr__ work"
  c = Pixmap(42, 43, t)
  assert "42x43" in str(c)
  assert "42x43" in repr(c)
  assert t.name in str(c)
  assert t.name in repr(c)

@alltypes()
def test_blit_with_offset_and_rotation(t):
  "Blit with various shifts and rotation"
  c1 = PixmapRand(19, 17, t)
  for r in range(4):
    for i in [0,1,2]:
      c2 = Pixmap(51-i, 25+i, t)
      c1.blit(2+i, 3+i, c2, 1+i, 4-i, w=2+3*i, h=13-i)
    c1.rotate_cw()

@alltypes()
def test_blit_vs_convert_to_RGB888(t):
  "Compare Blit vs Convert"
  if 'P' in t.name:
    raise SkipTest("Palette conversions are TODO")
  c = PixmapRand(42, 43, t, seed=0)
  c2a = PixmapRand(c.w, c.h, core.C.PIXEL_RGB888, seed=42)
  if 'A' in t.name:
    try:
      import gfxprim.gfx
    except ImportError:
      raise SkipTest("Could not import gfx for core test")
    c2a.gfx.fill(0)
  c2b = c.convert(core.C.PIXEL_RGB888)
  c.blit(0, 0, c2a, 0, 0, w=c.w, h=c.h)
  assert c2a == c2b

@alltypes()
def test_copy(t):
  "Copying works"
  c = PixmapRand(19, 43, t)
  c2 = c.copy(True)
  assert c == c2

@alltypes()
def test_rotate(t):
  "Rotations work (and LLL=R)"
  c = PixmapRand(42, 47, t)
  c2 = c.copy(True)
  assert c == c2
  c2.rotate_ccw()
  assert c != c2
  c.rotate_ccw()
  assert c == c2
  c2.rotate_ccw()
  assert c != c2
  c2.rotate_ccw()
  assert c != c2
  c2.rotate_ccw()
  assert c != c2
  c.rotate_cw()
  assert c == c2

@alltypes()
def test_subpixmap(t):
  "Subpixmap is sensible"
  c1a = PixmapRand(43, 51, t)
  c1b = PixmapRand(43, 51, t)
  assert c1a == c1b
  c2a = c1a.sub_pixmap(5, 7, 10, 9)
  c2b = c1b.sub_pixmap(5, 7, 10, 9)
  assert c2a == c2b

@alltypes()
def test_subpixmap_vs_blit(t):
  "Compare Subpixmap and Blit of a rectangle"
  c = PixmapRand(31, 21, t)
  c2a = c.sub_pixmap(5, 7, 15, 9)
  c2b = Pixmap(15, 9, t)
  c.blit(5, 7, c2b, 0, 0, w=15, h=9)
  assert c2a == c2b

@alltypes()
def test_blits_by_rect(t):
  "Blit defined by XYXY, XYX2Y2 and XYWH"
  c = PixmapRand(17, 13, t, seed=765)
  c2a = PixmapRand(16, 15, t, seed=4)
  c2b = PixmapRand(16, 15, t, seed=4)
  c2c = PixmapRand(16, 15, t, seed=4)
  c2d = PixmapRand(16, 15, t, seed=4)
  c2e = PixmapRand(16, 15, t, seed=4)
  assert c2a == c2b
  assert c2a == c2c
  assert c2a == c2d
  assert c2a == c2e
  assert c != c2a

  c.blit(3, 4, c2a, 5, 2, w=4, h=5)
  assert c2a != c2e
  c.blit(3, 4, c2b, 5, 2, sx2=6, sy2=8)
  assert c2b != c2e
  c.blit(3, 4, c2c, 5, 2, tx2=8, ty2=6)
  assert c2c != c2e
  core.c_core.gp_blit_xywh(c, 3, 4, 4, 5, c2d, 5, 2)
  assert c2d != c2e
  core.c_core.gp_blit_xyxy(c, 3, 4, 6, 8, c2e, 5, 2)

  assert c2a == c2b
  assert c2a == c2c
  assert c2a == c2d
  assert c2a == c2e


#      'RGBAToPixel',
#       'RGBToPixel',
#        'Resize',

