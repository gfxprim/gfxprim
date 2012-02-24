"""
Module wrapping GfxPrim backends.

BIG FAT WARNING
---------------
Accessing a context after its backend has ben freed will probably
end your (program's) world
"""

from . import backends_c

# Pull GP_Backend
Backend = backends_c.GP_Backend

# Constants module
from . import C

def _init(module):
  # Extend Context with convenience methods
  from ._extend_context import extend_context
  from ..core import Context
  extend_context(Context)

  # Extend GP_Backend with convenience methods
  from _extend_backend import extend_backend
  extend_backend(Backend)

  # Import some members from the SWIG module
  from ..utils import import_members
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  # Constants
  const_regexes = ['^GP_[A-Z0-9_]*$']
  import_members(backends_c, C, include=const_regexes, sub=strip_GP)

  # Functions
  import_members(backends_c, module, sub=strip_GP,
    exclude=const_regexes + [
      '^GP_Backend$',
      '^gfxprim$',
      '^\w+_swigregister$',
      '^_\w+$'])

_init(locals())
del _init
