"""
Module extending the Context class with .filter submodule.

Use as in "import gfxprim.filters; context_foo.filter.Resize(...)"
"""

# Import the SWIG wrapper
from . import c_filters

def _init(module):
  "Extend Context with filters submodule"

  from ..utils import extend_submodule
  from ..core import Context as _context

  # New Context submodule
  class FiltersSubmodule(object):
    def __init__(self, ctx):
      self.ctx = ctx

  _context._submodules['filters'] = FiltersSubmodule

  for name in ['Resize', 'ResizeAlloc',
               'Rotate90', 'Rotate90Alloc',
	       'Rotate180', 'Rotate180Alloc',
	       'Rotate270', 'Rotate270Alloc',
	       'MirrorH', 'MirrorHAlloc',
	       'MirrorV', 'MirrorVAlloc',
	       'Addition', 'Multiply', 'Difference', 'Max', 'Min',
	       'GaussianBlur', 'GaussianBlurAlloc',
	       'GaussianBlurEx', 'GaussianBlurExAlloc',
	       'GaussianNoiseAdd', 'GaussianNoiseAddAlloc',
	       'GaussianNoiseAddEx', 'GaussianNoiseAddExAlloc',
	       'Laplace', 'LaplaceAlloc',
	       'EdgeSharpening', 'EdgeSharpeningAlloc',
	       'Median', 'MedianAlloc', 'MedianEx', 'MedianExAlloc',
	       'Sigma', 'SigmaAlloc', 'SigmaEx', 'SigmaExAlloc',
	       'FloydSteinberg', 'FloydSteinbergAlloc',
	       'HilbertPeano', 'HilbertPeanoAlloc']:
    extend_submodule(FiltersSubmodule, name, c_filters.__getattribute__('GP_Filter' + name))

  # Imports from the SWIG module
  import re
  def strip_GP_Filter(s):
    return re.sub('^GP_Filter', '', s)

  # Import functions from the SWIG module
  from ..utils import import_members
  import_members(c_filters, module, sub=strip_GP_Filter,
      include=[
        '^GP_Filter.*Alloc',
        '^GP_Filter[A-Za-z0-9]*$',
        ])

_init(locals())
del _init
