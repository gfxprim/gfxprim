from .. import core

# HACK to allow gfxprim_loaders_c find gfxprim_core_c
def import_gfxprim_loaders_c_helper():
  from os.path import dirname
  import sys
  oldpath = sys.path[:]
  sys.path.append(dirname(__file__) + '/../core')
  from . import gfxprim_loaders_c as loaders_c
  sys.path = oldpath
  return loaders_c
loaders_c = import_gfxprim_loaders_c_helper()

# Extend Context with convenience methods
from . import extend_context
extend_context.extend_context_class(core.Context)
del extend_context
