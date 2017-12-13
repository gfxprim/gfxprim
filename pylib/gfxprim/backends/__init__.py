"""
Module wrapping GfxPrim backends.

BIG FAT WARNING
---------------
Accessing a pixmap after its backend has ben freed will probably
end your (program's) world
"""

from . import c_backends

# Pull gp_backend
backend = c_backends.gp_backend

def _init(module):

  # Extend gp_backend with convenience methods
  from ._extend_backend import extend_backend
  extend_backend(backend)

  # Imports from the SWIG module
  import re
  def strip_gp(s):
    return re.sub('^gp_', '', s)

  # Import some members from the SWIG module
  from ..utils import import_members
  import_members(c_backends, module, sub=strip_gp,
    exclude=[
      '^gp_backend$',
      '^gfxprim$',
      '^\w+_swigregister$',
      '^_\w+$'])

_init(locals())
del _init
