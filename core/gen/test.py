from pixeltype import *
from generators import *
import defs

c = []
h = []

h.append(r("""
/* TEMP def of base type */
typedef unsigned long int GP_Pixel;

/* helper macro to read bits */
/* WONDER: is 1ULL a good (fast) idea? */
#define GP_GET_BITS(offset, bits, val) ( ( (val)>>(offset) ) & ( (1ULL<<(bits)) - 1) ) 

/* helper macro to or-mask bits (set if zero beforehead), assuming val fits into bits */
#define GP_OR_BITS(offset, or, val) ( (val) |= ((bits)<<(offset)) )

/* helper macros to transfer s1-bit value to s2-bit value 
 * NOTE and WARNING: scaling is efficient and accurate for upscaling, 
 *                   but requires actual numebrs as the parameters !!! */
#define GP_SCALE_VAL(s1, s2, val) ( GP_SCALE_VAL_##s1##_##s2(val) )
{% for s1 in [1, 2, 4, 8] %}{% for s2 in [1, 2, 4, 8] -%}
{% if s2>s1 -%}
#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) ((val) * {{ multcoef(s1, s2) }})
{% else -%}
#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) ((val) >> {{ s1 - s2 }})
{% endif -%}
{% endfor %}{% endfor %}
\n""", multcoef = lambda s1,s2: hex(sum([1<<i*s1 for i in range(s2/s1)])) ))

# TODO:
#/* helper macro to set bits (clearing them first), size is bit-size of the type */
#define GP_SET_BITS(size, offset, bits, val) ( (val) &= ( ((1<<(bits))-1) - ((1<<(bits))-1))   ( (val)>>(offset) ) & ( (1<<(bits)) - 1) )
#"""]

for f in pixeltypes.values():
  h.append(str_description(f))
  gen_print(f, h, c)
  gen_get_chs(f, h, c)

gen_convert_to(pixeltypes['RGB565'], pixeltypes['RGBA8888'], h, c)
gen_convert_to(pixeltypes['RGBA8888'], pixeltypes['V2'], h, c)
gen_convert_to(pixeltypes['VA12'], pixeltypes['RGBA8888'], h, c)

print "/**** HEADER */\n"
print '\n'.join(h)
print "/**** CODE */\n#include<stdio.h>"
print '\n'.join(c)

