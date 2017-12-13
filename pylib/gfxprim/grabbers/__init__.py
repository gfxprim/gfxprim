"""
Module for GFXprim grabbers.
"""

# Import the SWIG wrapper
from . import c_grabbers
from ..utils import extend

def extend_grabber(_Grabber):
  """
  Extends _grabber class with convenience methods.
  Called once on module initialization.
  """
  @extend(_Grabber)
  def start(self):
    "Starts frame capture."
    c_grabbers.gp_grabber_start(self)

  @extend(_Grabber)
  def stop(self):
    "Stops frame capture."
    c_grabbers.gp_grabber_stop(self)

  @extend(_Grabber)
  def poll(self):
    "Polls for a frame."
    return c_grabbers.gp_grabber_poll(self)

# Pull gp_grabber
Grabber = c_grabbers.gp_grabber

def _init(module):
  # Extend gp_grabber with convenience methods
  extend_grabber(Grabber)

  # Imports from the SWIG module
  import re
  def strip_gp(s):
    return re.sub('^gp_', '', s)

  # Import some members from the SWIG module
  from ..utils import import_members
  import_members(c_grabbers, module, sub=strip_gp,
    exclude=[
      '^gfxprim$',
      '^\w+_swigregister$',
      '^_\w+$'])

_init(locals())
del _init
