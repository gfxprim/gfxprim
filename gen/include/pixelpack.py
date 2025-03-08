#
#  gfxprim.pixelpack
#
# The PixelPack describes how pixels are packed in a buffer.
#
# For pixel sizes that are smaller than a byte the up bit or down bit defines
# if we read the pixels in a byte from left to right or the other way. The more
# common is up bit but both of them are used in the wild.
#
# For pixels that occupy more than one byte we have to define endianity, the
# order in which are bytes taken from the buffer. The more usual way is to take
# them byte by byte from the pixel byte array which corresponds to the big
# endian. But for pixel types that have 16bit channels e.g. 16bpp grayscale the
# bytes has to be stored in the buffer in the machine order. Because of this and
# also because of speed optimizations the PixelPack defaults to machine
# endianity which defines pixel packings without _LE or _BE suffix.
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
# 2014-2025 - Cyril Hrubis <chrubis@ucw.cz>
#

from sys import byteorder

# Big endian
BE = "BE"
# Little endian
LE = "LE"
# Current machine endian
ME = "ME"

# Down bit
DB = "DB"
# Up bit
UB = "UB"

class PixelPack(object):
  def __init__(self, size, bit_order=None, suffix=None, endian=ME):
    self.size = size
    assert self.size >= 0
    assert endian in [BE, LE, ME]

    self.req_endian = endian
    self.endian_suffix = '_%s' % endian

    if self.req_endian == ME:
      if byteorder == 'little':
        self.endian = LE
      else:
        self.endian = BE
    else:
      self.endian = endian

    self.bit_order = bit_order
    assert self.bit_order in [None, DB, UB]
    assert (bit_order is not None) == self.needs_bit_order()

    self.suffix = suffix

    if not self.suffix:
      self.suffix = '%dBPP' % size

      if not self.is_me():
        self.suffix += self.endian_suffix

      if bit_order:
        self.suffix += '_%s' % bit_order

    self.pack = 'GP_PIXEL_PACK_' + self.suffix

    self.pack_id = size;

    if (self.bit_order == DB):
        self.pack_id += 0x80

    if (self.is_le()):
        self.pack_id += 0x40

  def is_le(self):
    return self.endian == LE

  def is_be(self):
    return self.endian == BE

  def is_me(self):
    return self.req_endian == ME


  def needs_bit_order(self):
    return (self.size % 8) != 0

  def byte_aligned(self):
    return (self.size % 8) == 0

  def description(self):
    if self.bit_order:
      return "pixel size %d, bit order %s, suffix %s" % (self.size,
        self.bit_order, self.suffix)
    else:
      return "pixel size %d, suffix %s" % (self.size, self.suffix)

