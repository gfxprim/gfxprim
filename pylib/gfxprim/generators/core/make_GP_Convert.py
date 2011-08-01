#!/usr/bin/python
#
# Script generating:
#
#  GP_Convert.gen.c, GP_Convert.gen.h
#  GP_Convert_Scale.gen.h
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
#

from gfxprim.generators.generator import *
from gfxprim.generators.pixeltype import *
from gfxprim.generators.core.gen_convert import *


@generator(CHeaderGenerator(name = 'GP_Convert_Scale.gen.h'),
           descr = 'Fast value scaling macros',
           authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def core_GP_Pixel_Scale_gen(h):
  h.rhead(
    "/* helper macros to transfer s1-bit value to s2-bit value\n"
    " * NOTE: efficient and accurate for both up- and downscaling,\n"
    " * WARNING: GP_SCALE_VAL requires constants numbers as first two parameters\n"
    " */\n"
    "#define GP_SCALE_VAL(s1, s2, val) ( GP_SCALE_VAL_##s1##_##s2(val) )\n\n"

    "{% for s1 in range(1,9) %}{% for s2 in range(1,9) %}"
      "{% if s2>s1 %}"
	"#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) (((val) * {{ multcoef(s1, s2) }}) >> {{ (-s2) % s1 }})\n"
      "{% else %}"
	"#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) ((val) >> {{ s1 - s2 }})\n"
      "{% endif %}"
    "{% endfor %}{% endfor %}", multcoef = lambda s1,s2: hex(sum([1<<(i*s1) for i in range((s2+s1-1)/s1)]))
    )


@generator(CHeaderGenerator(name = 'GP_Convert.gen.h'),
           CSourceGenerator(name = 'GP_Convert.gen.c'),
           descr = 'Convert PixelType values macros and functions',
           authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def core_GP_Convert_gen_h(h, c):
  h.rhead('#include "GP_Common.h"\n');
  h.rhead('#include "GP_Context.h"\n');
  h.rhead('#include "GP_Pixel.h"\n\n');

  c.rhead('#include "GP_Convert.h"\n\n');

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


@generator(CSourceGenerator(name = 'GP_Convert.test.gen.c'),
           descr = 'GP_Convert tests',
           authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def core_GP_Convert_gen_h(c):
  c.rhead('#include "GP_Tests.h"\n')
  c.rhead('#include "GP_Convert.h"\n')
  c.rhead('#include "GP_TestingCore.h"\n\n')
  c.rhead('GP_SUITE(GP_Convert)\n\n')
  for t in pixeltypes.values():
    if not t.is_palette() and t.number != 0:
      c.rbody(
	  'GP_TEST(GP_ConvertPixel_for_{{ t.name }}, "loop_start=0, loop_end=4")\n'
	  '{\n'
	  '	GP_Pixel p1 = GP_RandomColor(GP_PIXEL_{{ t.name }});\n'
	  '	GP_Pixel p2 = GP_ConvertPixel(p1, GP_PIXEL_{{ t.name }}, GP_PIXEL_RGBA8888);\n'
	  '	GP_Pixel p3 = GP_ConvertPixel(p2, GP_PIXEL_RGBA8888, GP_PIXEL_{{ t.name }});\n'
	  '	GP_CHECK_EqualColors(p1, GP_PIXEL_{{ t.name }}, p3, GP_PIXEL_{{ t.name }});\n'
	  '	GP_CHECK_EqualColors(p1, GP_PIXEL_{{ t.name }}, p2, GP_PIXEL_RGBA8888);\n'
	  '}\n'
	  'GP_ENDTEST\n\n', t=t)
      c.rbody(
	  'GP_TEST(WhiteStaysWhite_via_{{ t.name }}, "loop_start=0, loop_end=4")\n'
	  '{\n'
	  '	GP_Pixel p1 = GP_RGBToPixel(255, 255, 255, GP_PIXEL_RGB888);\n'
	  '	GP_Pixel p2 = GP_RGB888ToPixel(p1, GP_PIXEL_{{ t.name }});\n'
	  '	GP_Pixel p3 = GP_PixelToRGB888(p2, GP_PIXEL_{{ t.name }});\n'
	  '	GP_CHECK_EqualColors(p1, GP_PIXEL_RGB888, p3, GP_PIXEL_RGB888);\n'
	  '}\n'
	  'GP_ENDTEST\n\n', t=t)
      c.rbody(
	  'GP_TEST(BlackStaysBlack_via_{{ t.name }}, "loop_start=0, loop_end=4")\n'
	  '{\n'
	  '	GP_Pixel p1 = GP_RGBToPixel(0, 0, 0, GP_PIXEL_RGB888);\n'
	  '	GP_Pixel p2 = GP_RGB888ToPixel(p1, GP_PIXEL_{{ t.name }});\n'
	  '	GP_Pixel p3 = GP_PixelToRGB888(p2, GP_PIXEL_{{ t.name }});\n'
	  '	GP_CHECK_EqualColors(p1, GP_PIXEL_RGB888, p3, GP_PIXEL_RGB888);\n'
	  '}\n'
	  'GP_ENDTEST\n\n', t=t)
