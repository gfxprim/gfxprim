
# Import the SWIG wrapper

from . import c_core


# Constants module

from . import C


# Main Pixmap proxy (extended below)

Pixmap = c_core.gp_pixmap


def _init(module):
  """
  Extends _pixmap class with core module methods and properties
  for object-oriented usage.
  Called once on module initialization.
  """

  import re
  from ..utils import extend, extend_direct, add_swig_getmethod, add_swig_setmethod
  from ..utils import import_members

  ### PixelTypeDescription

  _ptdescr = c_core.gp_pixel_type_desc

  # String representation of a Pixmap

  @extend(_ptdescr, name='__str__')
  @extend(_ptdescr, name='__repr__')
  def ptdescr_str(self):
    return "<PixelTypeDescription %s>" % (self.name, )

  ### Pixmap

  _pixmap = module['Pixmap']

  # String representation

  @extend(_pixmap, name='__str__')
  @extend(_pixmap, name='__repr__')
  def pixmap_str(self):
    return "<Pixmap %dx%d %s (%dbpp), gp_pixmap %sowned, %s parent>" % (
      self.w, self.h, module['PixelTypes'][self.pixel_type].name, self.bpp,
      "" if self.thisown else "not ",
      "with" if self.parent else "no")

  # Equality

  @extend(_pixmap, name='__eq__')
  def PixmapEqual(self, other):
    "Compare Pixmaps - pixel types, sizes and data must match (gamma ignored)."
    if not isinstance(other, _pixmap):
      raise TypeError("Can only compare two Pixmaps.")
    return bool(c_core.gp_pixmap_equal(self, other))

  # Constructor

  def pixeltype_no(pixeltype):
    "Return pixel type number from the number or a PixelType instance"
    if isinstance(pixeltype, int):
      return pixeltype
    if isinstance(pixeltype, c_core.gp_pixel_type_desc):
      return pixeltype.type
    raise TypeError("Not a PixelType instance or number: %r", pixeltype)

  extend(_pixmap, name='__swig_init__')(_pixmap.__init__)

  @extend(_pixmap, name='__init__')
  def PixmapCreate(self, w, h, pixeltype):
    "Allocate a new w*h bitmap of given type."
    # Add "parent" attribute, pointing to a wrapper of the actual parent or None
    _pixmap.__swig_init__(self, w, h, pixeltype_no(pixeltype))
    self.parent = None

  # New instance methods

  @extend(_pixmap)
  def sub_pixmap(self, x, y, w, h):
    "Create a subpixmap (a rectangular view)."
    c = c_core.gp_sub_pixmap_alloc(self, x, y, w, h)
    c.parent = self
    return c

  @extend(_pixmap)
  def copy(self, withdata):
    "Copy the pixmap to a new pixmap. Pixel data are copie optionally."
    flags = c_core.GP_COPY_WITH_PIXELS if withdata else 0
    return c_core.gp_pixmap_copy(self, flags)

  @extend(_pixmap)
  def convert(self, target_type):
    """Converts pixmap to a different pixel type, allocates new pixmap.
    See gp_pixmap_convert_alloc() for details."""
    return c_core.gp_pixmap_convert_alloc(self, pixeltype_no(target_type))

  @extend(_pixmap)
  def to_byte_array(self):
    """Returns new Python ByteArray created from pixmap pixels. The
       array size is exactly pixmap.bytes_per_row * pixmap.h"""
    return c_core.gp_pixmap_to_byte_array(self)

  # Manipulation

  extend_direct(_pixmap, "putpixel", c_core.gp_putpixel,
      "Set a pixel value encoded according to pixmap PixelType. Clipped.")

  extend_direct(_pixmap, "getpixel", c_core.gp_getpixel,
      "Get a pixel value (encoded according to pixmap PixelType). Clipped.")

  extend_direct(_pixmap, "rotate_cw", c_core.gp_pixmap_rotate_cw,
      "Rotate Pixmap clockwise by changing the pixmap orientation.")

  extend_direct(_pixmap, "rotate_ccw", c_core.gp_pixmap_rotate_ccw,
      "Rotate Pixmap counter-clockwise by changing the pixmap orientation.")

  extend_direct(_pixmap, "resize", c_core.gp_pixmap_resize,
      "Resize the pixmap bitmap (reallocate). Fails on subpixmaps.")

  # Blit

  @extend(_pixmap)
  def blit(self, sx, sy, target, tx, ty, w=None, h=None, sx2=None, sy2=None,
           tx2=None, ty2=None):
    """Copy a rectangle from self to target. (sx,sy) and (tx,ty) define
    upper-left corners, rectangle size is given by (width, height), lower-right
    corner in source or lower-right corner in the target. Clipped."""
    assert sum([w is not None, sx2 is not None, tx2 is not None]) == 1
    assert sum([h is not None, sy2 is not None, ty2 is not None]) == 1
    if sx2 is not None:
      w = max(0, sx2 - sx + 1)
    if tx2 is not None:
      w = max(0, tx2 - tx + 1)
    if sy2 is not None:
      h = max(0, sy2 - sy + 1)
    if ty2 is not None:
      h = max(0, ty2 - ty + 1)
    if (w < 0) or (h < 0):
      raise ValueError("Size of blit rect must be non-negative.")
    return c_core.gp_blit_xywh_clipped(self, sx, sy, w, h, target, tx, ty)

  # Color conversions

  @extend(_pixmap)
  def rgb_to_pixel(self, r, g, b):
    "Convert RGB888 (values 0-255) to pixmap pixel type."
    return c_core.gp_rgb_to_pixel(int(r), int(g), int(b), self.pixel_type)

  @extend(_pixmap)
  def rgba_to_pixel(self, r, g, b, a):
    "Convert RGBA8888 (values 0-255) to pixmap pixel type."
    return c_core.gp_rgba_to_pixel(int(r), int(g), int(b), int(a), self.pixel_type)

  @extend(_pixmap)
  def fill(self, pixel):
    "Fills pixmap with given pixel value."
    return c_core.gp_fill(self, pixel)

  # Handle submodule methods such as pixmap.gfx.Line(...)
  _available_submodules = frozenset(['gfx', 'loaders', 'text', 'filters'])

  extend(_pixmap, name='_submodules')({})

  old__getattr__ = _pixmap.__getattr__
  def Pixmap__getattr__(self, name):
    if name in _pixmap._submodules:
      assert name in _available_submodules
      return _pixmap._submodules[name](self)
    if name in _available_submodules:
      raise RuntimeError("GfxPrim submodule '%s' not loaded" % name)
    return old__getattr__(self, name)
  _pixmap.__getattr__ = Pixmap__getattr__

  # Import constants into a separate module

  const_regexes = [
      '^GP_[A-Z0-9_]*$',
      '^GP_PIXEL_x[A-Z0-9_]*$']
  def strip_gp(s):
    return re.sub('^gp_|^GP_', '', s)
  import_members(c_core, C, include=const_regexes, sub=strip_gp)
  # every Pixmap also points to C for convenience
  extend(_pixmap, name='C')(C)

  # Arrays with pixel type info
  module['pixel_types'] = [c_core.gp_pixel_types_access(i)
                              for i in range(C.PIXEL_MAX)]

  module['pixel_types_dict'] = dict(((t.name, t) for t in module['pixel_types']))

  def pixel_type_by_name(name):
    "Return a pixel_type descriptor by name, raise KeyError if no such type exists."
    return module['pixel_types_dict'][name]
  module['pixel_type_by_name'] = pixel_type_by_name

  # Bulk import of functions
  import_members(c_core, module, sub=strip_gp,
      include=[
        '^gp_color.*$', # Might use trimming
        '^gp_[gs]et_debug_level$',
        '^gp_pixel_rgb.*$', # ...Lookup and ...Match
        '^gp_pixel_to_rgb.*$', # Needs love
        '^gp_rgb.*$', # Needs filtering
        '^gp_fill',
	])

  module['Convert'] = c_core.gp_pixmap_convert_alloc

_init(locals())
del _init

