import gfxprim_core_c as core
import gfxprim_loaders_c as loaders

class Context(object):
  def __init__(self, context, owns_GP_Context, parent=None):
    """Create new GP_Context wrapper.

    The wrapper is just a (read-only) proxy with 
    some conveniently mapped context methods.
    Direct access is possible through self.context.
    """
    # GP_Context struct (pre-wrapped by SWIG)
    self.context = context
    # call GP_ContextFree on del?
    self.owns_GP_Context = owns_GP_Context
    # Parent Context of a subcontext (if any)
    # It is important to hold a ref to parent because of GC
    self.parent = parent

  def __del__(self):
    print "Deleting %s (owns_GP_Context: %s)" % (self, self.owns_GP_Context)
    if self.owns_GP_Context:
      core.GP_ContextFree(self.context)

  @property
  def w(self):
    "Context width (transformed)"
    return core.GP_ContextW(self.context)

  @property
  def h(self):
    "Context height (transformed)"
    return core.GP_ContextH(self.context)

  @property
  def bpp(self):
    "Context bpp"
    return self.context.bpp

  def __str__(self):
    return "<Context %dx%d, %dbpp, GP_Context %sowned, %s parent>" % (
	self.w, self.h, self.bpp,
	"" if self.owns_GP_Context else "not ",
	"with" if self.parent else "no")
  __repr__ = __str__

  def subcontext(self, x, y, w, h):
    "Create a subcontext (rectangular view)."
    c = core.GP_ContextSubContext(self.context, None, x, y, w, h)
    if not c:
      raise Exception("Error creating subcontext")
    return type(self)(c, owns_GP_Context=True, parent=self)

  def save(self, filename, format=None):
    """Save the image in given format (or guess by the extension)

    Currently, JPG, PNG and P[BGP]M are supported, but not for all 
    context pixel types.
    """
    if not format:
      format = filename.rsplit('.', 1)[-1]
    format = format.lower()
    if format == 'jpg':
      res = loaders.GP_SaveJPG(filename, self.context, None)
    elif format == 'png':
      res = loaders.GP_SavePNG(filename, self.context, None)
    elif format == 'pbm':
      res = loaders.GP_SavePBM(filename, self.context, None)
    elif format == 'pgm':
      res = loaders.GP_SavePGM(filename, self.context, None)
    elif format == 'ppm':
      res = loaders.GP_SavePPM(filename, self.context, None)
    else:
      raise Exception("Format %r not supported.", format)
    if res != 0:
      raise Exception("Error saving %r (code %d)", filename, res)

  @classmethod
  def create(cls, w, h, pixeltype):
    "Allocate a new w*h bitmap of given type."

    pixeltype_no = pixeltype if isinstance(pixeltype, int) else 0 # !!!
    # TODO: actually accept a PixelType
    c = core.GP_ContextAlloc(w, h, pixeltype_no)
    return cls(c, owns_GP_Context=True)

  @classmethod
  def load(cls, filename):
    "Load image from given file, guess type."
    c = loaders.GP_LoadImage_SWIG(filename)
    if not c:
      raise Exception("Error loading %r", filename)
    return cls(c, owns_GP_Context=True)

