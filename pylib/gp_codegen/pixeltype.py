#
#  gfxprim.pixeltype - Module with PixelType descrition class
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
#

import re
from .pixelsize import PixelSize

class PixelType(object):
  """Representation of one GP_PixelType"""

  def __init__(self, name, pixelsize, chanslist):
    """`name` must be a valid C identifier
    `pixelsize` is an instance of PixelSize
    `chanslist` is a list of triplets describing individual channels as
      [ (`chan_name`, `bit_offset`, `bit_size`) ]
      where `chan_name` is usually one of: R, G, B,
      V (value, used for grayscale), A (opacity)
    """
    assert re.match('\A[A-Za-z][A-Za-z0-9_]*\Z', name)
    self.name = name
    self.chanslist = chanslist
    self.chans = dict() # { chan_name: (offset, size) }
    self.pixelsize = pixelsize

    # Verify channel bits for overlaps
    # also builds a bit-map of the PixelType
    self.bits = ['x'] * pixelsize.size
    for c in chanslist:
      assert c[0] not in self.chans.keys()
      self.chans[c[0]] = c
      for i in range(c[1], c[1] + c[2]):
        assert(i < self.pixelsize.size)
        assert(self.bits[i] == 'x')
        self.bits[i] = c[0]

  def valid_for_config(self, config):
    "Check PixelType compatibility with given GfxPrimConfig."

    # all types except UNKNOWN must have one of these sizes
    if not self.is_unknown():
      assert(self.pixelsize in config.pixelsizes)

  def __str__(self):
    return "<PixelType " + self.name + ">"

  def is_palette(self):
    return ('P' in self.chans)

  def is_unknown(self):
    return (self.name == "UNKNOWN")

  def is_rgb(self):
    for i in 'RGB':
      if i not in self.chans: return False
    return True

  def is_gray(self):
    return ('V' in self.chans)
 
  def is_cmyk(self):
    for i in 'CMYK':
      if i not in self.chans: return False
    return True

  def is_alpha(self):
    return ('A' in self.chans)

