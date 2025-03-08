#
#  gfxprim.pixeltype - Module with PixelType descrition class
#
# 2011      - Tomas Gavenciak <gavento@ucw.cz>
# 2013-2025 - Cyril Hrubis <metan@ucw.cz>
#

import re
from pixelpack import PixelPack

class PixelChannel(list):
  def __init__(self, quadruplet, idx):
    (name, offset, size, gamma_shift) = quadruplet
    # Create the list -> backward compatibility with triplets
    self.append(name)
    self.append(offset)
    self.append(size)
    # Add index (position in pixel from left)
    self.idx = idx
    # Add some convinience variables
    self.name = name
    self.off = offset
    self.size = size
    self.lin_size = size + gamma_shift
    # Shift ready to used in C
    self.C_shift = " << " + hex(offset)
    # Maximal channel value as an integer
    self.max = 2 ** size - 1
    # Maximal value as a C string
    self.C_max = hex(self.max)
    # Maximal linearized channel value as an integer
    self.lin_max = 2 ** (size + gamma_shift) - 1
    # Maximal linearized channel value as a C string
    self.C_lin_max = hex(self.lin_max)
    # Chanel bitmask as int
    self.mask = self.max * (2 ** offset)
    # Channel bitmas as hex string
    self.C_mask = hex(self.mask)
    # True if this channel is alpha channel
    self.is_alpha = name == 'A'

class PixelType(object):
  """Representation of one gp_pixel_type"""

  def __init__(self, name, pixelpack, chanslist):
    """`name` must be a valid C identifier
    `pixelpack` is an instance of PixelPack
    `chanslist` is a list of quadruplets describing individual channels as
      [ (`chan_name`, `bit_offset`, `bit_size`, `gamma_shift`) ]
      where `chan_name` is usually one of: R, G, B,
      V (value, used for grayscale), A (opacity)
      The gamma_shift says how many bits will a channel size grow when linearized
      with a gamma function.
    """
    assert re.match('\A[A-Za-z][A-Za-z0-9_]*\Z', name)
    self.name = name

    # Create channel list with convinience variables
    new_chanslist = []
    self.chan_names = []
    idx = 0
    for i in chanslist:
      new_chanslist.append(PixelChannel(i, idx))
      idx = idx + 1
      self.chan_names.append(i[0])
    self.chanslist = new_chanslist
    self.chans = dict() # { chan_name: (offset, size) }
    self.pixelpack = pixelpack
    # C enum as defined in GP_Pixel.gen.h
    self.C_type = "GP_PIXEL_" + self.name
    # Number of channels in the pixel
    self.chan_cnt = len(self.chanslist)

    # Verify channel bits for overlaps
    # also builds a bit-map of the PixelType
    self.bits = ['x'] * pixelpack.size
    for c in new_chanslist:
      assert c[0] not in self.chans.keys()
      self.chans[c[0]] = c
      for i in range(c[1], c[1] + c[2]):
        assert(i < self.pixelpack.size)
        assert(self.bits[i] == 'x')
        self.bits[i] = c[0]

  def valid_for_config(self, config):
    "Check PixelType compatibility with given GfxPrimConfig."

    # all types except UNKNOWN must have one of these sizes
    if not self.is_unknown():
      assert(self.pixelpack in config.pixelpacks)

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

