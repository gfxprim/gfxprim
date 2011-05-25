#!/usr/bin/python
#
# Script generating GP_Pixel.gen.c and GP_Pixel.gen.h
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 
#

from gfxprim.genutils import *
from gfxprim.pixeltype import *
from gfxprim.genutils import j2render as r
from gfxprim.core.gen_pixeltype import *

h = []
c = []

## Headers

gen_headers(h, c, 
  descr = "fast value scaling macros", 
  authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"],
  generator = __file__,
  hdef = "GP_PIXEL_SCALEVAL")

h.append(r(
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
  ))

if __name__ == '__main__':
  main_write(h, None)

