#!/usr/bin/python
# Script generating GP_Pixel.gen.c and GP_Pixel.gen.h
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 

from pixeltype import *
from gen_utils import *
from gen_pixeltype import *
import defs

h = []
c = []

## Headers

gen_headers(h, c, 
  descr = "pixel type definitions and functions", 
  authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"],
  generator = __file__,
  hdef = "GP_PIXEL_GEN_H")

c.append('#include <stdio.h>\n')
c.append('#include <GP.h>\n')
c.append('#include "GP_Pixel.h"\n')

## Enum of types

gen_GP_PixelType(h, c)

## PixelType parameters in C

gen_GP_PixelTypes(h, c)

## List of types

for t in pixeltypes.values():
  if t.name != 'UNKNOWN':
    h.append(str_start(t))
    h.append(str_description(t))
    c.append(str_start(t))
    c.append(str_description(t))

    gen_print(t, h, c)
    gen_get_chs(t, h, c)
    gen_get_pixel_addr(t, h, c)

# Per-bpp macros
for bpp in bitsizes:
  for bit_endian in bit_endians:
    if (bpp < 8) or (bit_endian == bit_endians[0]):
      gen_get_pixel_addr_bpp(bpp, get_size_suffix(bpp, bit_endian), h, c)

## Conversion macros

gen_convert_to(pixeltypes['RGB565'], pixeltypes['RGBA8888'], h, c)
gen_convert_to(pixeltypes['RGBA8888'], pixeltypes['V2'], h, c)
gen_convert_to(pixeltypes['VA12'], pixeltypes['RGBA8888'], h, c)

## Close the files

gen_footers(h, c)

## Write out!

if __name__ == '__main__':
    main_write(h, c)

