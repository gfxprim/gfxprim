from . import loaders_c

def _init(module):

  # Extend Context with convenience methods
  from _extend_context import extend_context
  from ..core import Context
  extend_context(Context)

  # Imports from the SWIG module
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  # Import functions from the SWIG module
  from ..utils import import_members
  import_members(loaders_c, module, sub=strip_GP,
    exclude=[
      '^\w+_swigregister$',
      '^gfxprim$',
      '^_\w+$'])

_init(locals())
del _init
