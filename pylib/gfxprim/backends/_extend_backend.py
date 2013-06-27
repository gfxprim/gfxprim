from ..utils import extend, add_swig_getmethod, add_swig_setmethod
from . import c_backends
from ..input import c_input

def extend_backend(_backend):
  """
  Extends _backend class with convenience methods.
  Called once on module initialization.
  """

  @extend(_backend, name='__str__')
  @extend(_backend, name='__repr__')
  def backend_str(self):
    return "<Backend \"%s\", GP_Backend %sowned>" % (
      self.name, "" if self.thisown else "not ")

  @extend(_backend)
  def Flip(self):
    "If display is buffered, this copies content of context onto display."
    return c_backends.GP_BackendFlip(self)

  @extend(_backend)
  def UpdateRect(self, x0, y0, x1, y1):
    "Update a rectangle on a buffered backend."
    return c_backends.GP_BackendUpdateRect(self, x0, y0, x1, y1)

  @extend(_backend)
  def Poll(self):
    "Poll the backend."
    c_backends.GP_BackendPoll(self)

  @extend(_backend)
  def PollEvent(self):
    "Poll the backend for events."
    ev = c_input.GP_Event()

    if c_backends.GP_BackendPollEvent(self, ev) != 0:
        return ev

    return None

  @extend(_backend)
  def Wait(self):
    "Waits for backend."
    c_backends.GP_BackendWait(self)

  @extend(_backend)
  def WaitEvent(self):
    "Waits for backend event."
    ev = c_input.GP_Event()

    c_backends.GP_BackendWaitEvent(self, ev)

    return ev

  @extend(_backend)
  def GetEvent(self):
    "Removes event from the top of the backend event queue."
    ev = c_input.GP_Event()

    if c_backends.GP_BackendGetEvent(self, ev) != 0:
      return ev

    return None

  @extend(_backend)
  def PeekEvent(self):
    "Returns, but not removes, the top of the backend event queue."
    ev = c_input.GP_Event()

    if c_backends.GP_BackendPeekEvent(self, ev) != 0:
      return ev

    return None

  @extend(_backend)
  def PutEventBack(self, ev):
    "Puts back event removed from the top of the backend event queue."
    c_backends.GP_BackendPutEventBack(self, ev)

  @extend(_backend)
  def EventsQueued(self):
    "Returns the number of events queued in the backend event queue."
    return c_backends.GP_BackendEventsQueued(self)

  @extend(_backend)
  def SetCaption(self, caption):
    "Set backend window caption (if possible)"
    return c_backends.GP_BackendSetCaption(self, caption)

  @extend(_backend)
  def Resize(self, w, h):
    "Resize backend window (if possible)"
    return c_backends.GP_BackendResize(self, w, h)

  @extend(_backend)
  def ResizeAck(self):
    "Acknowledge backend resize."
    return c_backends.GP_BackendResizeAck(self)
