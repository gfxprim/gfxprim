from . import c_filters

def _init(module):
  "Extend Context with filters submodule"

  # Imports from the SWIG module
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  # Import functions from the SWIG module
  from ..utils import import_members
  import_members(c_filters, module, sub=strip_GP,
      include=[
        '^GP_Filter.*Alloc',
        '^GP_Filter[A-Za-z0-9]*$',
        ])

_init(locals())
del _init
