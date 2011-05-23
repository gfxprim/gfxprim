#!/usr/bin/python
# Script generating GP_Pixel_Blit.gen.c and GP_Pixel_Blit.gen.h
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 

from pixeltype import *
from gen_blit import *
from gen_utils import *
import defs

h = []
c = []

## Headers

gen_headers(h, c, 
  descr = "specialized blit functions and macros", 
  authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"],
  generator = __file__,
  hdef = "GP_PIXEL_BLIT_GEN_H")

c.append('#include <stdio.h>\n')
c.append('#include <string.h>\n')
c.append('#include "GP_Pixel.h"\n')
c.append('#include "GP.h"\n')
c.append('#include "GP_Context.h"\n')
c.append('#include "GP_Blit.gen.h"\n')

for bpp in bitsizes:
  for bit_endian in bit_endians:
    if (bpp < 8) or (bit_endian == bit_endians[0]):
      gen_blit_same_t(bpp, get_size_suffix(bpp, bit_endian), h, c)

## Close the files

gen_footers(h, c)

## Write out!

if __name__ == '__main__':
  main_write(h, c)

