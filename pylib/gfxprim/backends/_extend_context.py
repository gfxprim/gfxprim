from ..utils import extend, add_swig_getmethod, add_swig_setmethod
from . import backends_c

def extend_context(_context):
  """
  Extends _context class with backend support.
  """
  extend(_context, name='backend')(None)
