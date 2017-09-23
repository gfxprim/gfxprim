
# Import the SWIG wrapper

from . import c_core


# Constants module

from . import C


# Main Pixmap proxy (extended below)

Pixmap = c_core.GP_Pixmap


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

  _ptdescr = c_core.GP_PixelTypeDescription

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
    return "<Pixmap %dx%d %s (%dbpp), GP_Pixmap %sowned, %s parent>" % (
      self.w, self.h, module['PixelTypes'][self.pixel_type].name, self.bpp,
      "" if self.thisown else "not ",
      "with" if self.parent else "no")

  # Equality

  @extend(_pixmap, name='__eq__')
  def PixmapEqual(self, other):
    "Compare Pixmaps - pixel types, sizes and data must match (gamma ignored)."
    if not isinstance(other, _pixmap):
      raise TypeError("Can only compare two Pixmaps.")
    return bool(c_core.GP_PixmapEqual(self, other))

  # Constructor

  def pixeltype_no(pixeltype):
    "Return pixel type number from the number or a PixelType instance"
    if isinstance(pixeltype, int):
      return pixeltype
    if isinstance(pixeltype, c_core.GP_PixelTypeDescription):
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
  def SubPixmap(self, x, y, w, h):
    "Create a subpixmap (a rectangular view)."
    c = c_core.GP_SubPixmapAlloc(self, x, y, w, h)
    c.parent = self
    return c

  @extend(_pixmap)
  def Copy(self, withdata):
    "Copy the pixmap to a new pixmap. Pixel data are copie optionally."
    flags = c_core.GP_COPY_WITH_PIXELS if withdata else 0
    return c_core.GP_PixmapCopy(self, flags)

  @extend(_pixmap)
  def Convert(self, target_type):
    """Converts pixmap to a different pixel type, allocates new pixmap.
    See GP_PixmapConvertAlloc() for details."""
    return c_core.GP_PixmapConvertAlloc(self, pixeltype_no(target_type))

  @extend(_pixmap)
  def ToByteArray(self):
    """Returns new Python ByteArray created from pixmap pixels. The
       array size is exactly pixmap.bytes_per_row * pixmap.h"""
    return c_core.GP_PixmapToByteArray(self)

  # Manipulation

  extend_direct(_pixmap, "PutPixel", c_core.GP_PutPixel,
      "Set a pixel value encoded according to pixmap PixelType. Clipped.")

  extend_direct(_pixmap, "GetPixel", c_core.GP_GetPixel,
      "Get a pixel value (encoded according to pixmap PixelType). Clipped.")

  extend_direct(_pixmap, "RotateCW", c_core.GP_PixmapRotateCW,
      "Rotate Pixmap clockwise by changing the pixmap orientation.")

  extend_direct(_pixmap, "RotateCCW", c_core.GP_PixmapRotateCCW,
      "Rotate Pixmap counter-clockwise by changing the pixmap orientation.")

  extend_direct(_pixmap, "Resize", c_core.GP_PixmapResize,
      "Resize the pixmap bitmap (reallocate). Fails on subpixmaps.")

  # Blit

  @extend(_pixmap)
  def Blit(self, sx, sy, target, tx, ty, w=None, h=None, sx2=None, sy2=None,
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
    return c_core.GP_BlitXYWH_Clipped(self, sx, sy, w, h, target, tx, ty)

  # Color conversions

  @extend(_pixmap)
  def RGBToPixel(self, r, g, b):
    "Convert RGB888 (values 0-255) to pixmap pixel type."
    return c_core.GP_RGBToPixel(int(r), int(g), int(b), self.pixel_type)

  @extend(_pixmap)
  def RGBAToPixel(self, r, g, b, a):
    "Convert RGBA8888 (values 0-255) to pixmap pixel type."
    return c_core.GP_RGBAToPixel(int(r), int(g), int(b), int(a), self.pixel_type)

  @extend(_pixmap)
  def Fill(self, pixel):
    "Fills pixmap with given pixel value."
    return c_core.GP_Fill(self, pixel)

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
  def strip_GP(s):
    return re.sub('^GP_', '', s)
  import_members(c_core, C, include=const_regexes, sub=strip_GP)
  # every Pixmap also points to C for convenience
  extend(_pixmap, name='C')(C)

  # Arrays with pixel type info
  module['PixelTypes'] = [c_core.GP_PixelTypes_access(i)
                              for i in range(C.PIXEL_MAX)]

  module['PixelTypesDict'] = dict(((t.name, t) for t in module['PixelTypes']))

  def PixelTypeByName(name):
    "Return a PixelType descriptor by name, raise KeyError if no such type exists."
    return module['PixelTypesDict'][name]
  module['PixelTypeByName'] = PixelTypeByName

  # Bulk import of functions
  import_members(c_core, module, sub=strip_GP,
      include=[
        '^GP_Color.*$', # Might use trimming
        '^GP_[GS]etDebugLevel$',
        '^GP_PixelRGB.*$', # ...Lookup and ...Match
        '^GP_PixelToRGB.*$', # Needs love
        '^GP_RGB.*$', # Needs filtering
        '^GP_Fill',
	])

  module['Convert'] = c_core.GP_PixmapConvertAlloc

_init(locals())
del _init

