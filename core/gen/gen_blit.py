# Module generating C source and headers for various PixelTypes
# - submodule for blit and friends
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 

from gen_utils import *

# WARN: assuming little-endian in sub-byte pixels order (probably)
def gen_blit_same_t(size, header, code):
  "Generate a function blitting the same type of pixel"
  "Only depends on bpp (bit size)"

  header.append(r(
    "\n/*** Blit preserving type, variant for {{ size }}bpp ***\n"
    " * Assumes the contexts to be of the right types and sizes\n"
    " * Ignores transformations and clipping */\n\n"
    "void GP_Blit_{{ size }}bpp(const GP_Context *c1, int x1, int y1, int w, int h,\n"
    "			GP_Context *c2, int x2, int y2);\n",
    size=size))

  code.append(r(
    "\n/*** Blit preservimg type, variant for {{ size }} bpp ***/\n"
    "void GP_Blit_{{ size }}bpp(const GP_Context *c1, int x1, int y1, int w, int h,\n"
    "			GP_Context *c2, int x2, int y2)\n"
    "{\n"
    "	if (unlikely(w == 0 || h == 0)) return;\n\n"
    "	/* Special case - copy whole line-block with one memcpy() */\n"
    "	if (x1 == 0 && x2 == 0 && w == c1->w && c1->w == c2->w &&\n"
    "		c1->bytes_per_row == c2->bytes_per_row) {\n"
    "		memcpy(c2->pixels + c2->bytes_per_row * y2,\n"
    "		       c1->pixels + c1->bytes_per_row * y1,\n"
    "		       c1->bytes_per_row * h);\n"
    "		return;\n"
    "	}\n\n"
    "{% if size>=8 %}" # memcpy() per line
    "	/* General case - memcpy() each horizontal line */\n"
    "	for (int i=0; i<h; i++)\n"
    "		memcpy(GP_PIXEL_ADDR_{{ size }}bpp(c2, x2, y2 + i), \n"
    "		       GP_PIXEL_ADDR_{{ size }}bpp(c1, x2, y2 + i),\n"
    "		       {{ size/8 }} * w);\n"
    "{% else %}" # subtle - rectangles may not be byte aligned in the same way
    "	/* Number of bits in the first byte */\n"
    "	int al1 = GP_PIXEL_ADDR_OFFSET(x1);\n"
    "	int al2 = GP_PIXEL_ADDR_OFFSET(x2);\n"
    "	if (al1 == al2) {\n"
    "		/* Number of bits in the last byte */\n"
    "		int end_al = GP_PIXEL_ADDR_OFFSET(x1 + w);\n"
    "		assert((w - al1 - end_al) % {{ ppb }} == 0);\n"
    "		int copy_size = (w - al1 - end_al) / {{ ppb }};\n"
    "		/* First and last byte incident to the line */\n"
    "		uint8_t *p1 = GP_PIXEL_ADDR_{{ size }}bpp(c1, x1, y1);\n"
    "		uint8_t *p2 = GP_PIXEL_ADDR_{{ size }}bpp(c2, x2, y2);\n"
    "		uint8_t *end_p1 = GP_PIXEL_ADDR_{{ size }}bpp(c1, x1 + w - 1, y1);\n"
    "		uint8_t *end_p2 = GP_PIXEL_ADDR_{{ size }}bpp(c2, x2 + w - 1, y2);\n"
    "		for (int i=0; i<h; i++) {\n"
    "			if (al1 != 0)\n"
    "				GP_SET_BITS(al1, 8-al1, *p2, GP_GET_BITS(al1, 8-al1, *p1));\n"
    "			memcpy(p2+(al1!=0), p1+(al1!=0), copy_size);\n"
    "			if (end_al != 0) {\n"
    "				GP_SET_BITS(0, end_al, *end_p2, GP_GET_BITS(0, end_al, *end_p1));\n"
    "			p1 += c1->bytes_per_row;\n"
    "			end_p1 += c1->bytes_per_row;\n"
    "			p2 += c2->bytes_per_row;\n"
    "			end_p2 += c2->bytes_per_row;\n"
    "		}\n"
#    "		/* Check for source strip inside just one byte */\n"
#    "		if (x1 / {{ ppb }} == (x1 + w - 1) / {{ ppb }}) {\n" 
#    "			int bwidth = w * {{ size }};\n"
#    "			uint8_t v1 = ((* GP_PIXEL_ADDR_{{ size }}bpp(c1, x1, y1)) <<\n"
#    "				GP_PIXEL_ADDR_OFFSET_{{ size }}bpp(x1)) >>\n"
#    "				GP_PIXEL_ADDR_OFFSET_{{ size }}bpp(x2);\n"
#    "			uint8_t *p2 = GP_PIXEL_ADDR_{{ size }}bpp(c2, x2, y2);\n"
#    "			"
#    "			GP_SET_BITS(
#    "			/// TODO: CHECK for narrow lines, these are special-case
    "	} else /* Different bit-alignment, can't use memcpy() */\n"
    "		GP_Blit_V{{ size }}(c1, x1, y1, w, h, c2, x2, y2);\n"
    "{% endif %}"
    "}\n", size=size, ppb=8/size))



def gen_blit_t(f1, f2, header, code):
  "Generate a macro blitting rectangle f1 to f2"
  "Does not (yet) convert between PAL formats and RGBVA"
  allowed_chansets = [ set(list(s)) for s in ['RGB', 'RGBA', 'V', 'VA'] ]
  if f1!=f2:
    assert(set(f1.chans.keys()) in allowed_chansets)
    assert(set(f2.chans.keys()) in allowed_chansets)

  header.append(r(
    "\n/*** Blit line {{ f1.name }} -> {{ f2.name }} ***\n"
    " * Assumes the contexts to be of the right types and sizes */\n\n"
    "void GP_Blit_{{ f1.name }}_{{ f2.name }}(const GP_Context *c1, int x1, int y2, int w, int h,\n"
    "							GP_Context *c2, int x2, int y2);\n",
      f1=f1, f2=f2))

