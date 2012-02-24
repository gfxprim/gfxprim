from ..utils import extend, add_swig_getmethod, add_swig_setmethod
from . import backends_c

def extend_context_class(_context_class):
  """
  Extends _context_class class with backend support.
  """
  extend(_context_class, name='backend')(None)
