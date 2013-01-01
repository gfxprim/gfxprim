### Helper imports and decorators

from random import Random

from gfxprim import core

__all__ = ["alltypes", "RandomizeContext", "ContextRand"]


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
        nf = lambda: f(t)
        nf.__name__ = f.__name__ + "_" + t.name
        nf.__doc__ = "%s<%s:%s>"% (
            f.__doc__ + " " if f.__doc__ else "",
            __name__, nf.__name__)
        globals()[nf.__name__] = nf
    return None
  return decorate


# Switch to alltypes_new_functions by default
alltypes = alltypes_new_functions

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

