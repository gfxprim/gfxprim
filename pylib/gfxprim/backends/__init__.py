"""
Module wrapping GfxPrim backends.

BIG FAT WARNING
---------------
Accessing a context after its backend has ben freed will probably
end your (program's) world
"""

# HACK to allow backends_c to find core_c
def import_backends_c_helper():
  from os.path import dirname
  import sys
  oldpath = sys.path[:]
  sys.path.append(dirname(__file__) + '/../core')
  from . import backends_c
  sys.path = oldpath
  return backends_c
backends_c = import_backends_c_helper()
del import_backends_c_helper

# Extend Context with convenience methods
from . import extend_context
from ..core import Context
extend_context.extend_context_class(Context)
del Context
del extend_context


# Pull GP_Backend
GP_Backend = backends_c.GP_Backend

# Extend GP_Backend with convenience methods
from . import extend_backend
extend_backend.extend_backend_class(GP_Backend)
del extend_backend

# Constants module
from . import C

# Import some members from the SWIG module
def import_helper(module):
  from ..utils import import_members

  # Constants
  const_regexes = ['^GP_[A-Z0-9_]*$']
  import_members(backends_c, C, include=const_regexes)

  # Functions
  import_members(backends_c, module,
    exclude=const_regexes + [
      '^\w+_swigregister$',
      '^_\w+$'])
import_helper(locals())
del import_helper
