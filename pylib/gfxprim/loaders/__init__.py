from . import c_loaders


def Load(filename, callback=None):
  "Load image from given file, guessing the type."
  c = c_loaders.GP_LoadImage(filename, callback)
  return c


def _init(module):
  "Extend Context with loaders submodule"

  from ..utils import extend, add_swig_getmethod, add_swig_setmethod
  from ..core import Context as _context

  class LoadersSubmodule(object):
    def __init__(self, ctx):
      self.ctx = ctx

  _context._submodules['loaders'] = LoadersSubmodule

  @extend(LoadersSubmodule)
  def Save(self, filename, callback=None):
    """Save the image, guessing the type from the extension.

    Generally, not all pixel types work with all formats.
    """
    c_loaders.GP_SaveImage(self.ctx, filename, callback)

  # Imports from the SWIG module
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  # Import functions from the SWIG module
  from ..utils import import_members
  import_members(c_loaders, module, sub=strip_GP,
      include=[
        '^GP_Load[A-Z]{3}.*',
        '^GP_Save[A-Z]{3}.*',
        '^GP_ListLoaders$',
        '^GP_LoadMetaData$',
        ])

_init(locals())
del _init
