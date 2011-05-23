# Module with PixelType descrition class
# 2011 - Tomas Gavenciak <gavento@ucw.cz>

# pixel format:
#   type
#   - PAL
#   - HSVRGBA
#   per chan
#     name, pos, size
#   bitwidth
#   name

import re


## *Global* dictionary of all pixeltypes { name : PixelType }
pixeltypes = {}

## *Global* set of all encountered channel names  
channels = set()

## Allowed bit-sizes of pixels
bitsizes = [1,2,4,8,16,24,32]

## bit_endian values
bit_endians = ['LE', 'BE']

## Create pixel-size suffix (16BPP or 4BPP_LE)
def get_size_suffix(bpp, bit_endian):
  assert bpp in bitsizes
  assert bit_endian in bit_endians
  size_suffix = '%dBPP' % (bpp)
  if bpp < 8:
    size_suffix += '_' + bit_endian
  return size_suffix


class PixelType(object):
  """Representation of one GP_PixelType"""
  def __init__(self, name, size, chanslist, number=None, bit_endian=None):
    """`name` must be a valid C identifier
    `size` is in bits, allowed are 1, 2, 4, 8, 16, 24, 32
    `bit_endian` is order of 1,2,4BPP pixels in a byte, either 'BE' or 'LE'
    `chanslist` is a list of triplets describing individual channels as 
     [ (`chan_name`, `bit_offset`, `bit_size`) ]
     `chan_name` is usually one of: R, G, B, V (value, used for grayscale), A (opacity) 
     `number` is auto-assigned (internal use only)
    """
    assert re.match('\A[A-Za-z][A-Za-z0-9_]*\Z', name)
    self.name = name 
    self.chanslist = chanslist 
    self.chans = dict() # { chan_name: (offset, size) }
    # all types except UNKNOWN=0 must have one of these sizes
    if number is not 0:
      assert(size in bitsizes) 
    self.size = size

    # bit_endian matters only for 1,2,4bpp
    if size>=8 and bit_endian is None:
      bit_endian = bit_endians[0]
    assert bit_endian in bit_endians
    self.bit_endian = bit_endian

    if self.size == 0:
      self.size_suffix = "INVALID"
    else:
      self.size_suffix = get_size_suffix(self.size, self.bit_endian)

    # Numbering from 1 
    if number is not None:
      self.number = number
    else:
      self.number = max([ptype.number for ptype in pixeltypes.values()] + [0]) + 1
  
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

    assert self.name not in pixeltypes.keys()
    pixeltypes[self.name] = self
    channels.update(set(self.chans.keys()))

  def __str__(self):
    return "<PixelType " + self.name + ">"

if 0 not in pixeltypes:
  PixelType("UNKNOWN", 0, [], bit_endian=bit_endians[0], number=0)
