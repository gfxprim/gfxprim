from ..utils import extend, add_swig_getmethod, add_swig_setmethod
from . import core_c

def extend_context(_context):
  """
  Extends _context class with core module methods and properties
  for object-oriented usage.
  Called once on module inicialization.
  """

  # Add "parent" attribute
  extend(_context, name='parent')(None)

  @extend(_context, name='__str__')
  @extend(_context, name='__repr__')
  def context_str(self):
    return "<Context %dx%d, %dbpp, GP_Context %sowned, %s parent>" % (
      self.w, self.h, self.bpp,
      "" if self.thisown else "not ",
      "with" if self.parent else "no")

  @extend(_context)
  def Subcontext(self, x, y, w, h):
    "Create a subcontext (rectangular view)."
    c = core_c.GP_ContextSubContext(self, None, x, y, w, h)
    c.parent = self
    return c

  @extend(_context)
  def Copy(self, withdata):
    "Copy the context to a new context. Pixel data are copie optionally."
    flags = core_c.GP_COPY_WITH_PIXELS if withdata else 0
    return core_c.GP_ContextCopy(self, flags)

  @extend(_context)
  def Convert(self, target_type):
    """Converts context to a different pixel type, allocates new context.
    See GP_ContextConvert() for details."""
    pixeltype_no = target_type ## TODO also accept PixelType
    return core_c.GP_ContextConvert(self, pixeltype_no)

  @extend(_context, name='Create')
  @staticmethod
  def Create(w, h, pixeltype):
    "Allocate a new w*h bitmap of given type."

    pixeltype_no = pixeltype if isinstance(pixeltype, int) else 0 # !!!
    # TODO: actually accept a PixelType
    c = core_c.GP_ContextAlloc(w, h, pixeltype_no)
    return c

  @extend(_context)
  def RGBToPixel(self, r, g, b):
    "Convert RGB888 (values 0-255) to context pixel type."
    return core_c.GP_RGBToPixel(int(r), int(g), int(b), self.pixel_type)

  @extend(_context)
  def RGBAToPixel(self, r, g, b):
    "Convert RGBA8888 (values 0-255) to context pixel type."
    return GP_RGBToPixel(r, g, b, self.pixel_type)
