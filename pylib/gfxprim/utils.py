"""
Utility methods for polishing SWIGified gfxprim.
"""

def extend(cls, name=None):
  "Decorator extending a class with a function"
  def decf(method):
    funname = name
    if not funname:
      funname = method.__name__
    type.__setattr__(cls, funname, method)
    return method
  return decf

def add_swig_getmethod(cls, name=None):
  "Decorator to add a property get method to a SWIG-defined class"
  def decf(method):
    propname = name
    if not propname:
      propname = method.__name__
    cls.__swig_getmethods__[propname] = method
  return decf

def add_swig_setmethod(cls, name=None):
  "Decorator to add a property set method to a SWIG-defined class"
  def decf(method):
    propname = name
    if not propname:
      propname = method.__name__
    cls.__swig_setmethods__[propname] = method
  return decf

def import_members(from_, to, include=[], exclude=[], sub=None):
  """Import members of `from_` to `to`. By default take all. If `exclude` is provided,
  use as a filter. If `include` is provided, ONLY include those.
  `include` and `exclude` are lists of regexes to match (include ^ and $)."""
  assert not (include and exclude)
  import re
  il = map(re.compile, include)
  el = map(re.compile, exclude)
  for name in dir(from_):
    try:
      o = from_[name]
    except TypeError:
      o = from_.__getattribute__(name)

    ok = True
    if il:
      ok = False
      for x in il:
        if x.match(name):
          ok = True
    if el:
      for x in el:
        if x.match(name):
          ok = False

    if ok:
      newname = name if sub == None else sub(name)
      try:
        to[newname] = o
      except TypeError:
        to.__setattr__(newname, o)

