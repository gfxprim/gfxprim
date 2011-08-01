#
#  gfxprim.pixeltype - Module with PixelType descrition class
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
#

import re
import os
import sys
import gfxprim
from gfxprim import die


def get_size_suffix(self.bpp, bit_endian):
  "Create pixel-size suffix (like 16BPP or 4BPP_LE)"

  assert bpp in self.sizes
  assert bit_endian in self.bit_endians
  size_suffix = '%dBPP' % (bpp)
  if bpp < 8:
    size_suffix += '_' + bit_endian
  return size_suffix


class PixelType(object):
  """Representation of one GP_PixelType"""

  def __init__(self, name, size, chanslist, bit_endian=None):
    """`name` must be a valid C identifier
    `size` is type bit-width
    `bit_endian` is required in 1,2,4 BPP types to determine the order of
       pixels in a byte, either 'BE' or 'LE'
    `chanslist` is a list of triplets describing individual channels as 
      [ (`chan_name`, `bit_offset`, `bit_size`) ]
      where `chan_name` is usually one of: R, G, B,
      V (value, used for grayscale), A (opacity)
    """
    assert re.match('\A[A-Za-z][A-Za-z0-9_]*\Z', name)
    self.name = name 
    self.chanslist = chanslist 
    self.chans = dict() # { chan_name: (offset, size) }
    self.size = size
    self.bit_endian = bit_endian

    if self.size == 0:
      self.size_suffix = "INVALID"
    else:
      self.size_suffix = get_size_suffix(self.size, self.bit_endian)

    # Verify channel bits for overlaps
    # also builds a bit-map of the PixelType
    self.bits = ['x']*size 
    for c in chanslist:
      assert c[0] not in self.chans.keys()
      self.chans[c[0]] = c
      for i in range(c[1],c[1]+c[2]):
	assert(i<self.size)
	assert(self.bits[i]=='x')
	self.bits[i] = c[0]

  def valid_for_config(self, config):
    "Check PixelType compatibility with given GfxPrimConfig."

    # all types except UNKNOWN=0 must have one of these sizes
    if self.name != "UNKNOWN":
      assert(self.size in config.sizes)

    # bit_endian matters only for non-multiple-of-8 bpp
    if size % 8 != 0 or bit_endian is not None:
      assert bit_endian in config.bit_endians

  def __str__(self):
    return "<PixelType " + self.name + ">"

  def is_palette(self):
    return ('P' in self.chans)

