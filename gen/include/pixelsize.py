#
#  gfxprim.pixelsize
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
# 2014 - Cyril Hrubis <chrubis@ucw.cz>
#

LE = "LE"
BE = "BE"

class PixelSize(object):
  def __init__(self, size, bit_endian=None, suffix=None):
    self.size = size
    assert self.size >= 0

    self.bit_endian = bit_endian
    assert self.bit_endian in [None, LE, BE]
    assert (bit_endian is not None) == self.needs_bit_endian()
    self.bit_endian_const = "GP_BIT_ENDIAN_" + (self.bit_endian or LE)

    self.suffix = suffix
    if not self.suffix:
      if self.size == 0:
        self.suffix = "INVALID"
      else:
        if bit_endian:
          self.suffix = '%dBPP_%s' % (size, bit_endian)
        else:
          self.suffix = '%dBPP' % (size,)

  def needs_bit_endian(self):
    return (self.size % 8) != 0

  def description(self):
    if self.bit_endian:
      return "pixel size %d, bit endian %s, suffix %s" % (self.size,
        self.bit_endian, self.suffix)
    else:
      return "pixel size %d, suffix %s" % (self.size, self.suffix)

