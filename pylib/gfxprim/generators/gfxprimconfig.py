#
#  gfxprimconfig.py - Class for (global) GfxPrim configuration
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>

import re, os, sys
  
class GfxPrimConfig(object):
  def __init__(self, pixel_type = None, pixel_size=None, sizes=None,
	       bit_endians=None, types=[]):

    self.pixel_type = pixel_type
    assert self.pixel_type
    assert isinstance(self.pixel_type, str)

    self.pixel_size = pixel_size
    assert isinstance(self.pixel_size, int)
    assert self.pixel_size % 8 == 0
    assert self.pixel_size > 0

    # Allowed bit-sizes of pixel types
    self.sizes = sizes
    assert isinstance(self.sizes, list)
    assert self.pixel_size in self.sizes
    assert all(( isinstance(i, int) and i > 0 and i <= self.pixel_size
      for i in self.sizes))

    # bit_endian values
    self.bit_endians = bit_endians
    assert isinstance(self.bit_endians, list)
    assert all(( i in ["LE", "BE"] for i in self.bit_endians))

    # Set of all encountered channel names
    self.channels = set()

    # Dictionary of all pixeltypes { name : PixelType }
    self.types_dict = {}
    # List of all PixelTypes in order. "Unknown" must be first.
    self.types = []
  
    self.add_pixeltype(PixelType("UNKNOWN", 0, [], bit_endian=bit_endians[0]))
    for t in types:
      self.add_pixeltype(t)

  def add_pixeltype(self, pixeltype):
    "Add a PixelType and check its against the config"

    assert pixeltype not in self.types
    self.types.append(pixeltype)
    assert pixeltype.name not in self.types_dict
    self.types_dict[pixeltype.name] = pixeltype
    self.channels.update(set(pixeltype.chans.keys()))
    try:
      pixeltype.check_config(self)
    except AssertionError:
      sys.stderr.write("Error checking PixelType %s\n" % pixeltype.name)

