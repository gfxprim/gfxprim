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
  def Start(self):
    "Starts frame capture."
    c_grabbers.GP_GrabberStart(self)
  
  @extend(_Grabber)
  def Stop(self):
    "Stops frame capture."
    c_grabbers.GP_GrabberStop(self)
  
  @extend(_Grabber)
  def Poll(self):
    "Polls for a frame."
    return c_grabbers.GP_GrabberPoll(self)

# Pull GP_Grabber
Grabber = c_grabbers.GP_Grabber

def _init(module):
  # Extend GP_Grabber with convenience methods
  extend_grabber(Grabber)  
  
  # Imports from the SWIG module
  import re
  def strip_GP(s):
    return re.sub('^GP_', '', s)

  # Import some members from the SWIG module
  from ..utils import import_members
  import_members(c_grabbers, module, sub=strip_GP,
    exclude=[
      '^gfxprim$',
      '^\w+_swigregister$',
      '^_\w+$'])

_init(locals())
del _init
