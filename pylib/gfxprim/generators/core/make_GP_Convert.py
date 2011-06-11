#!/usr/bin/python
#
# Script generating:
#
#  GP_Convert.gen.c, GP_Convert.gen.h
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 
#

from gfxprim.generators.generator import *
from gfxprim.generators.pixeltype import *
from gfxprim.generators.core.gen_convert import *


@generator(CHeaderGenerator(name = 'GP_Convert.gen.h'),
           CSourceGenerator(name = 'GP_Convert.gen.c'),
           descr = 'Convert PixelType values macros and functions\n'
                   'Do not include directly, use GP_Convert.h',
           authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def core_GP_Convert_gen_h(h, c):
  h.rhead('#include "GP_Common.h"\n');
  h.rhead('#include "GP_Context.h"\n');
  h.rhead('#include "GP_Pixel.h"\n\n');
  
  h.rhead('#include "GP_Common.h"\n');
  c.rhead('#include "GP_Pixel.h"\n\n');

  ## two base types for conversions
  for bt in [pixeltypes['RGB888'], pixeltypes['RGBA8888']]:
    gen_fixedtype_to_type(bt, h, c)
    gen_type_to_fixedtype(bt, h, c)
    ## Conversion macros
    for t in pixeltypes.values():
      if not t.is_palette() and t.number != 0:
	gen_convert_to(t, bt, h, c)
	gen_convert_to(bt, t, h, c)
  
  ## Just experimental conversion macros
  gen_convert_to(pixeltypes['RGB565'], pixeltypes['RGBA8888'], h, c)
  gen_convert_to(pixeltypes['RGBA8888'], pixeltypes['V2'], h, c)
  gen_convert_to(pixeltypes['VA12'], pixeltypes['RGBA8888'], h, c)
