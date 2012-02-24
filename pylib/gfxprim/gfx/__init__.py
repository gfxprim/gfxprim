from . import gfx_c

def _init(module):

  # Imports from the SWIG module
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  # Import functions from the SWIG module
  from ..utils import import_members
  import_members(gfx_c, module, sub=strip_GP,
    exclude=[
      '.*_Raw',
      '^\w+_swigregister$',
      '^core_c$',
      '^_\w+$'])

_init(locals())
del _init
