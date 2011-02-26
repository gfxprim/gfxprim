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

class PixelType(object):
  """Representation of one GP_PixelType"""
  def __init__(self, name, size, chanslist):
    """`name` must be a valid C identifier
    `size` is in bits, allowed are 1, 2, 4, 8, 16, 24, 32
    `chanslist` is a list of triplets describing individual channels as 
     [ (`chan_name`, `bit_offset`, `bit_size`) ]
     `chan_name` is usually one of: R, G, B, V (value, used for grayscale), A (opacity) 
    """
    assert re.match('\A[A-Za-z][A-Za-z0-9_]*\Z', name)
    self.name = name 
    self.chanslist = chanslist 
    self.chans = dict() # { chan_name: (offset, size) }
    assert(size in [1,2,4,8,16,24,32]) 
    self.size = size
    # Numbering from 1 (0 for invalid type)
    self.number = min([ptype.number for ptype in pixeltypes.values()] + [0]) + 1
  
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
