"""
Module wrapping GfxPrim backends.

BIG FAT WARNING
---------------
Accessing a context after its backend has ben freed will probably
end your (program's) world
"""

from . import c_backends

# Pull GP_Backend
Backend = c_backends.GP_Backend

def _init(module):

  # Extend GP_Backend with convenience methods
  from ._extend_backend import extend_backend
  extend_backend(Backend)

  # Imports from the SWIG module
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  # Import some members from the SWIG module
  from ..utils import import_members
  import_members(c_backends, module, sub=strip_GP,
    exclude=[
      '^GP_Backend$',
      '^gfxprim$',
      '^\w+_swigregister$',
      '^_\w+$'])

_init(locals())
del _init
