"""
Module wrapping GfxPrim Input.
"""

from . import input_c

# Constants module
from . import C

def _init(module):
  # Import some members from the SWIG module
  from ..utils import import_members
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  # Constants
  const_regexes = ['^GP_[A-Z0-9_]*$']
  import_members(input_c, C, include=const_regexes, sub=strip_GP)

  # Functions
  import_members(input_c, module, sub=strip_GP,
    exclude=const_regexes + [
      '^gfxprim$',
      '^\w+_swigregister$',
      '^_\w+$'])

_init(locals())
del _init
