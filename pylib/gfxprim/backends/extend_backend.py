from ..utils import extend, add_swig_getmethod, add_swig_setmethod
from . import backends_c

def extend_backend_class(_backend_class):
  """
  Extends _backend_class class with convenience methods.
  Called once on module inicialization.
  """

  @extend(_backend_class, name='__str__')
  @extend(_backend_class, name='__repr__')
  def backend_str(self):
    return "<Backend \"%s\", GP_Backend %sowned>" % (
      self.name, "" if self.thisown else "not ")

  @extend(_backend_class)
  def Flip(self):
    "If display is buffered, this copies content of context onto display."
    return backends_c.GP_BackendFlip(self)

  @extend(_backend_class)
  def UpdateRect(self, rect):
    "Update a rectangle on a buffered backend."
    return backends_c.GP_BackendUpdateRect(self, rect[0], rect[1], rect[2], rect[3])

  @extend(_backend_class)
  def Poll(self):
    "Poll the backend for events."
    return backends_c.GP_BackendPoll(self)

