
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
  from ..utils import extend, add_swig_getmethod, add_swig_setmethod
  from ..utils import import_members
  _context = module['Context']

  # String representation

  @extend(_context, name='__str__')
  @extend(_context, name='__repr__')
  def context_str(self):
    return "<Context %dx%d, %dbpp, GP_Context %sowned, %s parent>" % (
      self.w, self.h, self.bpp,
      "" if self.thisown else "not ",
      "with" if self.parent else "no")

  # Creation

  def pixeltype_no(pixeltype):
    "Return pixel type number from the number or a PixelType instance"
    if isinstance(pixeltype, int):
      return pixeltype
    if isinstance(pixeltype, c_core.GP_PixelTypeDescription):
      return pixeltype.type
    raise TypeError("Not a PixelType instance or number: %r", pixeltype)

  @extend(_context, name='Create')
  @staticmethod
  def Create(w, h, pixeltype):
    "Allocate a new w*h bitmap of given type."
    # Add "parent" attribute, pointing to a wrapper of the actual parent or None
    c = c_core.GP_ContextAlloc(w, h, pixeltype_no(pixeltype))
    c.parent = None
    return c

  @extend(_context)
  def Subcontext(self, x, y, w, h):
    "Create a subcontext (rectangular view)."
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
    See GP_ContextConvert() for details."""
    return c_core.GP_ContextConvert(self, pixeltype_no(target_type))

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

  # Bulk import of functions - TODO: only import specified
  import_members(c_core, module, sub=strip_GP,
    exclude=const_regexes + [
      '^GP_Blit\w+$',
      '^GP_Context\w+$',
      '^GP_PixelSNPrint\w+$',
      '^GP_WritePixels\w+$',
      '.*_Raw.*',
      '^\w+_swigregister$',
      '^cvar$',
      '^_\w+$'])

_init(locals())
del _init

"""
TODO: on entering any func from Python, set up error reporting on GP_ABORT
(raise an exception, restore stack and return, possibly longjmp?
(or just force python trace?)

!! LIST of symbols to implement

! Pixeltype table and objects

GP_PixelTypes

! IN Context class

GP_ContextAlloc
GP_ContextResize
GP_ContextConvertAlloc
GP_ContextPrintInfo
GP_ContextRotateCCW
GP_SubContextAlloc
GP_ContextConvert
GP_ContextRotateCW
GP_ContextFree
GP_ContextInit
GP_SubContext
GP_ContextCopy
GP_PixelAddrOffset

! ?

# GP_GammaRelease
# GP_GammaCopy
# GP_GammaAcquire

! ?

# GP_DebugPrint
# GP_SetDebugLevel
# GP_GetDebugLevel

! Color conversion

GP_RGBA8888ToPixel
GP_RGB888ToPixel
GP_PixelToRGB888
GP_PixelToRGBA8888
GP_ColorNameToPixel

GP_PixelTypeByName - reimplement

# GP_ColorToPixel
# GP_PixelRGBMatch
# GP_ColorLoadPixels
# GP_PixelRGBLookup
# GP_ColorNameToColor
# GP_ColorToColorName

! IN Context class

Blit - reimplement with keyword args, no raw

# GP_BlitXYXY
# GP_BlitXYXY_Fast
# GP_BlitXYWH
# GP_BlitXYWH_Clipped
# GP_BlitXYXY_Clipped
# GP_BlitXYXY_Raw_Fast
# GP_BlitXYWH_Raw
# GP_BlitXYXY_Raw

! Do not want

# GP_WritePixels_1BPP_LE
# GP_WritePixels_2BPP_LE
# GP_WritePixels_4BPP_LE
# GP_WritePixels1bpp
# GP_WritePixels2bpp
# GP_WritePixels4bpp
# GP_WritePixels8bpp
# GP_WritePixels16bpp
# GP_WritePixels18bpp
# GP_WritePixels32bpp
# GP_WritePixels24bpp

! IN Context - basic drawing

GP_PutPixel
GP_GetPixel

! ?

# GP_NrThreads
# GP_NrThreadsSet
# GP_ProgressCallbackMP
# SWIG_exception
"""

