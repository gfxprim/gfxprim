from . import core_c

Context = core_c.GP_Context

# Constants
from . import C

def _init(module):

  # Extend Context with convenience methods
  from ._extend_context import extend_context
  extend_context(Context)

  # Import some members from the SWIG module
  import re
  from ..utils import import_members

  # Constants
  const_regexes = [
      '^GP_[A-Z0-9_]*$',
      '^GP_PIXEL_x[A-Z0-9_]*$']
  def strip_GP(s):
    return re.sub('^GP_', '', s)
  import_members(core_c, C, include=const_regexes, sub=strip_GP)

  # Functions
  import_members(core_c, module, sub=strip_GP,
    exclude=const_regexes + [
      '^GP_Blit\w+$',
      '^GP_Context\w+$',
      '^GP_PixelSNPrint\w+$',
      '^GP_WritePixels\w+$',
      '.*_Raw.*',
      '^\w+_swigregister$',
      '^cvar$',
      '^_\w+$'])

_init(locals())
del _init
