### Helper imports and decorators

from random import Random

from gfxprim import core

__all__ = ["alltypes", "for_each_case",
           "RandomizeContext", "ContextRand"]


def alltypes_generator(_filter=None):
  def decorate(f):
    def gen():
      for t in core.PixelTypes[1:]:
        if (_filter is None) or _filter(t):
          yield f, t
    gen.__name__ = f.__name__
    return gen
  return decorate


def alltypes_new_functions(_filter=None):
  def decorate(f):
    for t in core.PixelTypes[1:]:
      if (_filter is None) or _filter(t):
        nf = (lambda tt: (lambda: f(tt)))(t)
        nf.__name__ = f.__name__ + "_" + t.name
        nf.__module__ = f.__module__
        nf.__doc__ = "%s<%s:%s>"% (
            f.__doc__ + " " if f.__doc__ else "",
            nf.__module__, nf.__name__)
        f.__globals__[nf.__name__] = nf
    return None
  return decorate

# Switch to alltypes_new_functions by default
#alltypes = alltypes_new_functions


def alltypes(_filter=None):
  """
  Creates one test for each PixelType (except INVALID).
  The pixeltype is given to the test function and the name
  is appended to its name and docstring.
  """
  flt = lambda n, t: _filter(t)
  return for_each_case(dict([(t.name, t) for t in core.PixelTypes[1:]]),
                       givename=False, _filter=(flt if _filter else None))


def for_each_case(cases, givename=True, _filter=None):
  """
  Creates one test for each of `cases`.
  
  Cases is either list of strings or or string dict (with any values).
  The test is then given (name) for list or (name, value) for dict,
  or just (value) if givename=False.

  Optional _filter is called with (name) or (name, value) for dict to
  determine which values to use (ret. True -> include).
  """
  def decorate(f):
    for n in cases:
      assert isinstance(n, str)
      if isinstance(cases, dict):
        if givename:
          nf = (lambda nn, nv: (lambda: f(nn, nv)))(n, cases[n])
        else:
          nf = (lambda nv: (lambda: f(nv)))(cases[n])
      else: # assume a list or a tuple
        nf = (lambda nn: (lambda: f(nn)))(n)
      nf.__name__ = f.__name__ + "_" + n
      nf.__module__ = f.__module__
      nf.__doc__ = "%s<%s:%s>"% (
          f.__doc__ + " " if f.__doc__ else "",
          nf.__module__, nf.__name__)
      f.__globals__[nf.__name__] = nf
    return None
  return decorate


### core.Context helpers

def ContextRand(w, h, t, seed=None):
  "Return new Context(w, h, t) filled with RandomizeContext(c, seed)"
  c = core.Context(w, h, t)
  RandomizeContext(c, seed)
  return c

def RandomizeContext(c, seed=None):
  """Fill Context with pseudorandom data.

  The default seed is computed from size and type number.
  """

  if seed is None:
    seed = c.w + (1 << c.h) * c.pixel_type
  r = Random(seed)
  for x in range(c.w):
    for y in range(c.h):
      p = r.randint(0, (1 << c.bpp) - 1)
      c.PutPixel(x, y, p)
      assert c.GetPixel(x, y) == p

