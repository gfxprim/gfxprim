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
    return "<Backend \"%s\", gp_backend %sowned>" % (
      self.name, "" if self.thisown else "not ")

  @extend(_backend)
  def flip(self):
    "If display is buffered, this copies content of pixmap onto display."
    return c_backends.gp_backend_flip(self)

  @extend(_backend)
  def update_rect(self, x0, y0, x1, y1):
    "Update a rectangle on a buffered backend."
    return c_backends.gp_backend_update_rect(self, x0, y0, x1, y1)

  @extend(_backend)
  def poll(self):
    "Poll the backend."
    c_backends.gp_backend_poll(self)

  @extend(_backend)
  def poll_event(self):
    "Poll the backend for events."
    ev = c_input.gp_event()

    if c_backends.gp_backend_poll_event(self, ev) != 0:
        return ev

    return None

  @extend(_backend)
  def wait(self):
    "Waits for backend."
    c_backends.gp_backend_wait(self)

  @extend(_backend)
  def wait_event(self):
    "Waits for backend event."
    ev = c_input.gp_event()

    c_backends.gp_backend_wait_event(self, ev)

    return ev

  @extend(_backend)
  def get_event(self):
    "Removes event from the top of the backend event queue."
    ev = c_input.gp_event()

    if c_backends.gp_backend_get_event(self, ev) != 0:
      return ev

    return None

  @extend(_backend)
  def peek_event(self):
    "Returns, but not removes, the top of the backend event queue."
    ev = c_input.gp_event()

    if c_backends.gp_backend_peek_event(self, ev) != 0:
      return ev

    return None

  @extend(_backend)
  def put_event_back(self, ev):
    "Puts back event removed from the top of the backend event queue."
    c_backends.gp_backend_put_event_back(self, ev)

  @extend(_backend)
  def events_queued(self):
    "Returns the number of events queued in the backend event queue."
    return c_backends.gp_backend_events_queued(self)

  @extend(_backend)
  def set_caption(self, caption):
    "Set backend window caption (if possible)"
    return c_backends.gp_backend_set_caption(self, caption)

  @extend(_backend)
  def resize(self, w, h):
    "Resize backend window (if possible)"
    return c_backends.gp_backend_resize(self, w, h)

  @extend(_backend)
  def resize_ack(self):
    "Acknowledge backend resize."
    return c_backends.gp_backend_resize_ack(self)
