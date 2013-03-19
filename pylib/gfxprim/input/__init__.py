"""
Module wrapping GfxPrim Input.
"""

from . import c_input

def _init(module):
  # Import some members from the SWIG module
  from ..utils import import_members
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  # Functions
  import_members(c_input, module, sub=strip_GP,
    exclude=[
      '^gfxprim$',
      '^\w+_swigregister$',
      '^_\w+$'])

_init(locals())
del _init
