#!/usr/bin/python
#
# Script generating GP_Pixel.gen.c and GP_Pixel.gen.h
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 
#

from gfxprim.generators.generator import *
#from gfxprim.generators.core.gen_pixeltype import *

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

@generator(CSourceGenerator(name='GP_Pixel.gen.c'),
	   CHeaderGenerator(name='GP_Pixel.gen.h'),
	   descr = 'pixel type definitions and functions',
	   authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def core_GP_Pixel_gen(c, h):
  c.rhead("CCCCCC")
  h.rhead("HHHHHH")





