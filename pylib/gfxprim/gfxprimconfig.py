#
#  gfxprim.generators.gfxprimconfig - Class for (global) GfxPrim configuration
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>

import os
import logging as log
from gfxprim.pixeltype import PixelType
from gfxprim.pixelsize import PixelSize

class GfxPrimConfig(object):
  def __init__(self, pixel_type = None, pixel_size=None, pixelsizes=None,
	       pixeltypes=None):
    """Initialize GfxPrim code generation config

    pixel_type: name of C type for a pixel value
    pixel_size: number of bits of pixel_type
    pixelsizes: list of generated and allowed PixelSizes
    pixeltypes: list of generated PixelTypes, not incl. UNKNOWN
    """

    self.pixel_type = pixel_type
    assert self.pixel_type
    assert isinstance(self.pixel_type, str)

    self.pixel_size = pixel_size
    assert isinstance(self.pixel_size, int)
    assert self.pixel_size % 8 == 0
    assert self.pixel_size > 0

    # Allowed bit-sizes of pixel types
    self.pixelsizes = pixelsizes
    assert isinstance(self.pixelsizes, list)
    assert self.pixel_size in [i.size for i in self.pixelsizes]
    assert all((i.size <= self.pixel_size for i in self.pixelsizes))

    # Set of all encountered channel names
    self.channels = set()

    # Dictionary of all pixeltypes { name : PixelType }
    self.pixeltypes_dict = {}
    # List of all PixelTypes in order. "Unknown" MUST be first.
    self.pixeltypes = []

    self.add_pixeltype(PixelType("UNKNOWN", PixelSize(0), []))
    if pixeltypes:
      for t in pixeltypes:
        self.add_pixeltype(t)

  def add_pixeltype(self, pixeltype):
    "Add a PixelType and check its against the config"

    assert pixeltype not in self.pixeltypes
    self.pixeltypes.append(pixeltype)
    assert pixeltype.name not in self.pixeltypes_dict
    self.pixeltypes_dict[pixeltype.name] = pixeltype
    self.channels.update(set(pixeltype.chans.keys()))
    try:
      pixeltype.valid_for_config(self)
    except AssertionError:
      log.error("Error checking PixelType %s\n" % pixeltype.name)

