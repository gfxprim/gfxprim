#
#  gfxprim.pixelpack
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
# 2014 - Cyril Hrubis <chrubis@ucw.cz>
#

DB = "DB"
UB = "UB"

class PixelPack(object):
  def __init__(self, size, bit_order=None, suffix=None):
    self.size = size
    assert self.size >= 0

    self.bit_order = bit_order
    assert self.bit_order in [None, DB, UB]
    assert (bit_order is not None) == self.needs_bit_order()

    self.suffix = suffix
    if not self.suffix:
      if self.size == 0:
        self.suffix = "INVALID"
      else:
        if bit_order:
          self.suffix = '%dBPP_%s' % (size, bit_order)
        else:
          self.suffix = '%dBPP' % (size,)

    self.pack = 'GP_PIXEL_PACK_' + self.suffix

    self.pack_id = size;
    if (self.bit_order == DB):
        self.pack_id = size + 0x70

  def needs_bit_order(self):
    return (self.size % 8) != 0

  def description(self):
    if self.bit_order:
      return "pixel size %d, bit order %s, suffix %s" % (self.size,
        self.bit_order, self.suffix)
    else:
      return "pixel size %d, suffix %s" % (self.size, self.suffix)

