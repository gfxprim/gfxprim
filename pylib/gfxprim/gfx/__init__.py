from . import gfx_c

from . import C

def _init(module):


  # Imports from the SWIG module
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  # Import constants from the SWIG module
  from ..utils import import_members
  const_regexes = [
      '^GP_[A-Z0-9_]*$',
      '^GP_PIXEL_x[A-Z0-9_]*$']
  import_members(gfx_c, C, include=const_regexes, sub=strip_GP)

  # Import functions from the SWIG module
  import_members(gfx_c, module, sub=strip_GP,
    exclude=const_regexes + [
      '.*_Raw',
      '^\w+_swigregister$',
      '^core_c$',
      '^_\w+$'])

_init(locals())
del _init
