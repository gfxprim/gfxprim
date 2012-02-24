from ..utils import extend, add_swig_getmethod, add_swig_setmethod
from . import backends_c

def extend_backend(_backend):
  """
  Extends _backend class with convenience methods.
  Called once on module inicialization.
  """

  @extend(_backend, name='__str__')
  @extend(_backend, name='__repr__')
  def backend_str(self):
    return "<Backend \"%s\", GP_Backend %sowned>" % (
      self.name, "" if self.thisown else "not ")

  @extend(_backend)
  def Flip(self):
    "If display is buffered, this copies content of context onto display."
    return backends_c.GP_BackendFlip(self)

  @extend(_backend)
  def UpdateRect(self, rect):
    "Update a rectangle on a buffered backend."
    return backends_c.GP_BackendUpdateRect(self, rect[0], rect[1], rect[2], rect[3])

  @extend(_backend)
  def Poll(self):
    "Poll the backend for events."
    return backends_c.GP_BackendPoll(self)

