# Module generating C source and headers for get/putpixel
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
# 2011 - Cyril Hrubis <metan@ucw.cz>

from gfxprim.generators.utils import *

def gen_get_pixel_addr_bpp(size, size_suffix, header):
  "Generate GP_PIXEL_ADDR_<size_suffix> and _OFFSET_<size_suffix> macros"
  bit_endian = size_suffix[-2:]
  if size < 8:
    assert bit_endian in ['LE', 'BE']
  header.rbody(
    "/* macro to get address of pixel in a {{ size_suffix }} context */\n"
    "#define GP_PIXEL_ADDR_{{ size_suffix }}(context, x, y) \\\n"
    "        ((context)->pixels + (context)->bytes_per_row * (y) + {{ size//8 }} * (x))\n"
    "/* macro to get bit-offset of pixel in {{ size_suffix }} context */\n"
    "{% if size >= 8 %}"
      "#define GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x) (0)\n"
    "{% else %}" # bit_endian matters
      "{% if bit_endian=='LE' %}"
        "#define GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x) \\\n"
	"	(((x) % {{ 8//size }}) * {{ size }})\n"
      "{% else %}"
        "#define GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x) \\\n"
	"	({{ 8-size }} - ((x) % {{ 8//size }}) * {{ size }})\n"
      "{% endif %}"
    "{% endif %}",
    size=size, size_suffix=size_suffix, bit_endian=bit_endian)

def gen_getpixel_bpp(size, size_suffix, header):
  "Generate code for GetPixel_Raw_xBPP, no clipping or transform. "
  "Only depends on bpp (bit size), size_suffix must be "
  "of form 8BPP, 2BPP_LE and the like."
  header.rbody(
    "\n/*** GP_GetPixel for {{ size_suffix }} ***/\n"
    "static inline GP_Pixel GP_GetPixel_Raw_{{ size_suffix }}(const GP_Context *c, int x, int y)\n"
    "{\n"
    "	return GP_GET_BITS(GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x), {{ size }},\n"
    "		*(GP_PIXEL_ADDR_{{ size_suffix}}(c, x, y)));\n"
    "}\n\n", size=size, size_suffix=size_suffix)

def gen_putpixel_bpp(size, size_suffix, header):
  "Generate code for PutPixel_Raw_xBPP, no clipping or transform. "
  "Only depends on bpp (bit size), size_suffix must be "
  "of form 8BPP, 2BPP_LE and the like."
  header.rbody(
    "\n/*** GP_PutPixel for {{ size_suffix }} ***/\n"
    "static inline void GP_PutPixel_Raw_{{ size_suffix }}(GP_Context *c, int x, int y, GP_Pixel p)\n"
    "{\n"
    "	GP_SET_BITS(GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x) , {{ size }},\n"
    "		*(GP_PIXEL_ADDR_{{ size_suffix}}(c, x, y)), p);\n"
    "}\n\n", size=size, size_suffix=size_suffix)
