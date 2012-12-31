
# Import the SWIG wrapper

from . import c_core


# Constants module

from . import C


# Main Context proxy (extended below)

Context = c_core.GP_Context


def _init(module):
  """
  Extends _context class with core module methods and properties
  for object-oriented usage.
  Called once on module initialization.
  """

  import re
  from ..utils import extend, extend_direct, add_swig_getmethod, add_swig_setmethod
  from ..utils import import_members

  ### PixelTypeDescription

  _ptdescr = c_core.GP_PixelTypeDescription

  # String representation of 

  @extend(_ptdescr, name='__str__')
  @extend(_ptdescr, name='__repr__')
  def ptdescr_str(self):
    return "<PixelTypeDescription %s>" % (self.name, )

  ### Context

  _context = module['Context']

  # String representation

  @extend(_context, name='__str__')
  @extend(_context, name='__repr__')
  def context_str(self):
    return "<Context %dx%d %s (%dbpp), GP_Context %sowned, %s parent>" % (
      self.w, self.h, module['PixelTypes'][self.pixel_type].name, self.bpp,
      "" if self.thisown else "not ",
      "with" if self.parent else "no")

  # Equality

  @extend(_context, name='__eq__')
  def ContextEqual(self, other):
    "Compare Contexts - pixel types, sizes and data must match (gamma ignored)."
    if not isinstance(other, _context):
      raise TypeError("Can only compare two Contexts.")
    return bool(c_core.GP_ContextEqual(self, other))

  # Constructor

  def pixeltype_no(pixeltype):
    "Return pixel type number from the number or a PixelType instance"
    if isinstance(pixeltype, int):
      return pixeltype
    if isinstance(pixeltype, c_core.GP_PixelTypeDescription):
      return pixeltype.type
    raise TypeError("Not a PixelType instance or number: %r", pixeltype)

  extend(_context, name='__swig_init__')(_context.__init__)

  @extend(_context, name='__init__')
  def ContextCreate(self, w, h, pixeltype):
    "Allocate a new w*h bitmap of given type."
    # Add "parent" attribute, pointing to a wrapper of the actual parent or None
    _context.__swig_init__(self, w, h, pixeltype_no(pixeltype))
    self.parent = None

  # New instance methods

  @extend(_context)
  def SubContext(self, x, y, w, h):
    "Create a subcontext (a rectangular view)."
    c = c_core.GP_SubContextAlloc(self, x, y, w, h)
    c.parent = self
    return c

  @extend(_context)
  def Copy(self, withdata):
    "Copy the context to a new context. Pixel data are copie optionally."
    flags = c_core.GP_COPY_WITH_PIXELS if withdata else 0
    return c_core.GP_ContextCopy(self, flags)

  @extend(_context)
  def Convert(self, target_type):
    """Converts context to a different pixel type, allocates new context.
    See GP_ContextConvertAlloc() for details."""
    return c_core.GP_ContextConvertAlloc(self, pixeltype_no(target_type))

  # Manipulation

  extend_direct(_context, "PutPixel", c_core.GP_PutPixel,
      "Set a pixel value encoded according to context PixelType. Clipped.")

  extend_direct(_context, "GetPixel", c_core.GP_GetPixel,
      "Get a pixel value (encoded according to context PixelType). Clipped.")

  extend_direct(_context, "RotateCW", c_core.GP_ContextRotateCW,
      "Rotate Context clockwise by changing the context orientation.")

  extend_direct(_context, "RotateCCW", c_core.GP_ContextRotateCCW,
      "Rotate Context counter-clockwise by changing the context orientation.")

  extend_direct(_context, "Resize", c_core.GP_ContextResize,
      "Resize the context bitmap (reallocate). Fails on subcontexts.")

  # Blit

  @extend(_context)
  def Blit(self, sx, sy, target, tx, ty, w=None, h=None, sx2=None, sy2=None,
           tx2=None, ty2=None):
    """Copy a rectangle from self to target. (sx,sy) and (tx,ty) define
    upper-left corners, rectangle size is given by (width, height), lower-right
    corner in source or lower-right corner in the target. Clipped."""
    assert sum([w is not None, sx2 is not None, tx2 is not None]) == 1
    assert sum([h is not None, sy2 is not None, ty2 is not None]) == 1
    if sx2 is not None:
      w = max(0, sx2 - sx)
    if tx2 is not None:
      w = max(0, tx2 - tx)
    if sy2 is not None:
      h = max(0, sy2 - sy)
    if ty2 is not None:
      h = max(0, ty2 - ty)
    return c_core.GP_BlitXYWH_Clipped(self, sx, sy, w, h, target, tx, ty)

  # Color conversions

  @extend(_context)
  def RGBToPixel(self, r, g, b):
    "Convert RGB888 (values 0-255) to context pixel type."
    return c_core.GP_RGBToPixel(int(r), int(g), int(b), self.pixel_type)

  @extend(_context)
  def RGBAToPixel(self, r, g, b, a):
    "Convert RGBA8888 (values 0-255) to context pixel type."
    return c_core.GP_RGBAToPixel(int(r), int(g), int(b), int(a), self.pixel_type)

  # Handle submodule methods such as context.gfx.Line(...)
  _available_submodules = frozenset(['gfx', 'loaders', 'text', 'filters'])

  extend(_context, name='_submodules')({})

  old__getattr__ = _context.__getattr__
  def Context__getattr__(self, name):
    if name in _context._submodules:
      assert name in _available_submodules
      return _context._submodules[name](self)
    if name in _available_submodules:
      raise RuntimeError("GfxPrim submodule '%s' not loaded" % name)
    return old__getattr__(self, name)
  _context.__getattr__ = Context__getattr__

  # Import constants into a separate module

  const_regexes = [
      '^GP_[A-Z0-9_]*$',
      '^GP_PIXEL_x[A-Z0-9_]*$']
  def strip_GP(s):
    return re.sub('^GP_', '', s)
  import_members(c_core, C, include=const_regexes, sub=strip_GP)
  # every Context also points to C for convenience
  extend(_context, name='C')(C)

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
        #'^GP_ProgressCallback.*$', # Needs work
        ])

_init(locals())
del _init

