from . import c_loaders

def load(filename, callback=None):
  "Load image from given file, guessing the type."
  c = c_loaders.gp_load_image(filename, callback)
  return c

def _init(module):
  "Extend Pixmap with loaders submodule"

  from ..utils import extend, add_swig_getmethod, add_swig_setmethod
  from ..core import Pixmap as _pixmap

  class LoadersSubmodule(object):
    def __init__(self, ctx):
      self.ctx = ctx

  _pixmap._submodules['loaders'] = LoadersSubmodule

  @extend(LoadersSubmodule)
  def save(self, filename, callback=None):
    """Save the image, guessing the type from the extension.

    Generally, not all pixel types work with all formats.
    """
    c_loaders.gp_save_image(self.ctx, filename, callback)

  @extend(LoadersSubmodule)
  def save_png(self, filename, callback=None):
    """Save the image as PNG.

    Generally, not all pixel types work with all formats.
    """
    c_loaders.gp_save_png(self.ctx, filename, callback)

  @extend(LoadersSubmodule)
  def save_jpg(self, filename, callback=None):
    """Save the image as JPEG.

    Generally, not all pixel types work with all formats.
    """
    c_loaders.gp_save_jpg(self.ctx, filename, callback)

  @extend(LoadersSubmodule)
  def save_bmp(self, filename, callback=None):
    """Save the image as BMP.

    Generally, not all pixel types work with all formats.
    """
    c_loaders.gp_save_bmp(self.ctx, filename, callback)

  @extend(LoadersSubmodule)
  def save_pbm(self, filename, callback=None):
    """Save the image as PBM.

    Generally, not all pixel types work with all formats.
    """
    c_loaders.gp_save_pbm(self.ctx, filename, callback)

  @extend(LoadersSubmodule)
  def save_pgm(self, filename, callback=None):
    """Save the image as PGM.

    Generally, not all pixel types work with all formats.
    """
    c_loaders.gp_save_pgm(self.ctx, filename, callback)

  @extend(LoadersSubmodule)
  def save_ppm(self, filename, callback=None):
    """Save the image as PPM.

    Generally, not all pixel types work with all formats.
    """
    c_loaders.gp_save_ppm(self.ctx, filename, callback)

  @extend(LoadersSubmodule)
  def save_pnm(self, filename, callback=None):
    """Save the image as PNM.

    Generally, not all pixel types work with all formats.
    """
    c_loaders.gp_save_pnm(self.ctx, filename, callback)

  @extend(LoadersSubmodule)
  def save_tiff(self, filename, callback=None):
    """Save the image as TIFF.

    Generally, not all pixel types work with all formats.
    """
    c_loaders.gp_save_tiff(self.ctx, filename, callback)

  # Imports from the SWIG module
  import re
  def strip_gp(s):
    return re.sub('^gp_|^GP_', '', s)

  # Import functions from the SWIG module
  from ..utils import import_members
  import_members(c_loaders, module, sub=strip_gp,
      include=[
        '^gp_load[A-Z,0-9]{3}.*',
        '^gp_save[A-Z,0-9]{3}.*',
        '^gp_loaders_lists$',
        '^gp_load_meta_data$',
        ])

_init(locals())
del _init
