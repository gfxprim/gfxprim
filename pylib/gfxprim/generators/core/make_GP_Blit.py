#!/usr/bin/python
#
# Generators for GP_Pixel_Blit.gen.c and GP_Pixel_Blit.gen.h
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
#

from gfxprim.generators.generator import *
from gfxprim.generators.pixeltype import *
from gfxprim.generators.core.gen_pixeltype import *
from gfxprim.generators.core.gen_blit import *

@generator(CHeaderGenerator(name='GP_Blit.gen.h'),
           CSourceGenerator(name='GP_Blit.gen.c'),
	   descr = 'specialized blit functions and macros',
	   authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def core_GP_Blit_gen(h, c):
  c.rhead(
    '#include <stdio.h>\n'
    '#include <string.h>\n'
    '#include "GP_Pixel.h"\n'
    '#include "GP.h"\n'
    '#include "GP_Context.h"\n'
    '#include "GP_Blit.h"\n'
    )

  for bpp in bitsizes:
    for bit_endian in bit_endians:
      if (bpp < 8) or (bit_endian == bit_endians[0]):
	gen_blit_same_t(bpp, get_size_suffix(bpp, bit_endian), h, c)

