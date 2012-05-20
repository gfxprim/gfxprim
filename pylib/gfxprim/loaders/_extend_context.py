from ..utils import extend, add_swig_getmethod, add_swig_setmethod
from . import loaders_c

def extend_context(_context):
  """
  Extends _context class with loader module methods for calling convenience.
  Called once on loaders module inicialization.
  """

  @extend(_context, name='load')
  @staticmethod
  def Load(filename):
    "Load image from given file, guess type."
    c = loaders_c.GP_LoadImage_Wrap(filename)
    return c

  @extend(_context)
  def Save(self, filename, format=None, callback=None):
    """Save the image in given format (or guess it from the extension)

    Currently, JPG, PNG and P[BGP]M are supported, but not for all 
    context pixel types.
    """
    if not format:
      format = filename.rsplit('.', 1)[-1]
    format = format.lower()
    if format == 'jpg':
      res = loaders_c.GP_SaveJPG(self, filename, callback)
    elif format == 'png':
      res = loaders_c.GP_SavePNG(self, filename, callback)
    elif format == 'pbm':
      res = loaders_c.GP_SavePBM(filename, self, callback)
    elif format == 'pgm':
      res = loaders_c.GP_SavePGM(filename, self, callback)
    elif format == 'ppm':
      res = loaders_c.GP_SavePPM(filename, self, callback)
    else:
      raise Exception("Format %r not supported.", format)
    if res != 0:
      raise Exception("Error saving %r (code %d)", filename, res)

