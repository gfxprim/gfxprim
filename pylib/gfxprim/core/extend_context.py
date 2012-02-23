from ..utils import extend, add_swig_getmethod, add_swig_setmethod
from . import gfxprim_core_c as core_c

def extend_context_class(_context_class):
  """
  Extends _context_class class with core module methods and properties
  for object-oriented usage.
  Called once on module inicialization.
  """

  # Add "parent" attribute
  extend(_context_class, name='parent')(None)

  @extend(_context_class, name='__str__')
  @extend(_context_class, name='__repr__')
  def context_str(self):
    return "<Context %dx%d, %dbpp, GP_Context %sowned, %s parent>" % (
      self.w, self.h, self._bpp,
      "" if self.thisown else "not ",
      "with" if self.parent else "no")

  @add_swig_getmethod(_context_class)
  def w(self):
    return core_c.GP_ContextW(self)

  @add_swig_getmethod(_context_class)
  def h(self):
    return core_c.GP_ContextH(self)

  @extend(_context_class)
  def subcontext(self, x, y, w, h):
    "Create a subcontext (rectangular view)."
    c = core_c.GP_ContextSubContext(self, None, x, y, w, h)
    c.parent = self
    return c

  @extend(_context_class)
  def copy(self, withdata):
    "Copy the context to a new context. Pixel data are copie optionally."
    flags = core_c.GP_COPY_WITH_PIXELS if withdata else 0
    return core_c.GP_ContextCopy(self, flags)

  @extend(_context_class)
  def convert(self, target_type):
    """Converts context to a different pixel type, allocates new context.
    See GP_ContextConvert() for details."""
    pixeltype_no = target_type ## TODO also accept PixelType
    return core_c.GP_ContextConvert(self, pixeltype_no)

  @extend(_context_class, name='create')
  @staticmethod
  def create(w, h, pixeltype):
    "Allocate a new w*h bitmap of given type."

    pixeltype_no = pixeltype if isinstance(pixeltype, int) else 0 # !!!
    # TODO: actually accept a PixelType
    c = core_c.GP_ContextAlloc(w, h, pixeltype_no)
    return c

