#!/usr/bin/python
#
# Script generating:
#
#  GP_Pixel.gen.c, GP_Pixel.gen.h
#  GP_Pixel_Scale.gen.h
#  GP_Pixel_Access.gen.h
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 
#

from gfxprim.generators.generator import *
from gfxprim.generators.pixeltype import *
from gfxprim.generators.core.gen_pixeltype import *
from gfxprim.generators.core.gen_getputpixel import *

@generator(CHeaderGenerator(name = 'GP_Pixel_Scale.gen.h'),
           descr = 'Fast value scaling macros\nDo not include directly, use GP_Pixel.h',
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
	   descr = 'Pixel type definitions and functions\nDo not include directly, use GP_Pixel.h',
	   authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def core_GP_Pixel_gen(h, c):
  c.rhead(
    '#include <stdio.h>\n'
    '#include "GP_Pixel.h"\n\n')

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
      gen_create(t, h, c)


@generator(CHeaderGenerator(name = 'GP_GetPutPixel.gen.h'),
           descr = 'Access pixel bytes, Get and PutPixel\nDo not include directly, use GP_Pixel.h',
           authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def core_GP_Pixel_Scale_gen(h):
  h.rhead('#include "GP_Common.h"\n\n');
  h.rhead('struct GP_Context;\n\n');
  # Per-bpp adress/offset macros
  for bpp in bitsizes:
    for bit_endian in bit_endians:
      if (bpp < 8) or (bit_endian == bit_endians[0]):
	gen_get_pixel_addr_bpp(bpp, get_size_suffix(bpp, bit_endian), h)
  
  # Per-bpp adress/offset macros
  for bpp in bitsizes:
    for bit_endian in bit_endians:
      if (bpp < 8) or (bit_endian == bit_endians[0]):
	gen_getpixel_bpp(bpp, get_size_suffix(bpp, bit_endian), h)
	gen_putpixel_bpp(bpp, get_size_suffix(bpp, bit_endian), h)

