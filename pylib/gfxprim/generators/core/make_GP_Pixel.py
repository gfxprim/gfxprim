#!/usr/bin/python
#
# Script generating GP_Pixel.gen.c and GP_Pixel.gen.h
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 
#

from gfxprim.generators.generator import *
from gfxprim.generators.pixeltype import *
from gfxprim.generators.core.gen_pixeltype import *

@generator(CHeaderGenerator(name = 'GP_Pixel_Scale.gen.h'),
           descr = 'fast value scaling macros',
           authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def core_GP_Pixel_Scale_gen(h):
  h.rhead(
    "/* helper macros to transfer s1-bit value to s2-bit value\n"
    " * NOTE: efficient and accurate for both up- and downscaling,\n"
    " * WARNING: GP_SCALE_VAL requires constants numebrs as first two parameters\n"
    " */\n"
    "#define GP_SCALE_VAL(s1, s2, val) ( GP_SCALE_VAL_##s1##_##s2(val) )\n\n"

    "{% for s1 in [1, 2, 4, 8] %}{% for s2 in [1, 2, 4, 8] %}"
      "{% if s2>s1 %}"
	"#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) ((val) * {{ multcoef(s1, s2) }})\n"
      "{% else %}"
	"#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) ((val) >> {{ s1 - s2 }})\n"
      "{% endif %}"
    "{% endfor %}{% endfor %}", multcoef = lambda s1,s2: hex(sum([1<<i*s1 for i in range(s2/s1)]))
    )

@generator(CHeaderGenerator(name='GP_Pixel.gen.h'),
           CSourceGenerator(name='GP_Pixel.gen.c'),
	   descr = 'pixel type definitions and functions',
	   authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def core_GP_Pixel_gen(h, c):
  c.rhead(
    '#include <stdio.h>\n'
    '#include <GP.h>\n'
    '#include "GP_Pixel.h"\n')

  ## Enum of types
  gen_GP_PixelType(h, c)

  ## PixelType parameters in C
  gen_GP_PixelTypes(h, c)

  ## List of types
  for t in pixeltypes.values():
    if t.name != 'UNKNOWN':
      h.rbody(str_start(t))
      h.rbody(str_description(t))
      c.rbody(str_start(t))
      c.rbody(str_description(t))

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

