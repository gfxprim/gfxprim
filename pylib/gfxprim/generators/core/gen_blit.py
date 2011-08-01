# Module generating C source and headers for various PixelTypes
# - submodule for blit and friends
# 2011 - Tomas Gavenciak <gavento@ucw.cz>

from gfxprim.generators.utils import *


## all generated direct blits, for generating GP_Blit() and others
generated_blits = []


# TODO: adapt for both bit-endianness (in-byte prefix and suffix)
# WARN: assuming little-endian in sub-byte pixels order (probably)
def gen_blit_same_t(size, size_suffix, header, code):
  """Generate a function blitting the same type of pixel.
  Only depends on bpp (bit size), size_suffix must be
  of form 8BPP, 2BPP_LE and the like.
  """

  header.rbody(
    "\n/*** Blit preserving type, variant for {{ size_suffix }} ***\n"
    " * Assumes the contexts to be of the right types and sizes\n"
    " * Ignores transformations and clipping */\n\n"
    "void GP_Blit_{{ size_suffix }}(const GP_Context *c1, GP_Coord x1, GP_Coord y1, GP_Size w, GP_Size h,\n"
    "			GP_Context *c2, GP_Coord x2, GP_Coord y2);\n",
    size=size, size_suffix=size_suffix)

  code.rbody(
    "\n/*** Blit preservimg type, variant for {{ size_suffix }} ***/\n"
    "void GP_Blit_{{ size_suffix }}(const GP_Context *c1, GP_Coord x1, GP_Coord y1, GP_Size w, GP_Size h,\n"
    "			GP_Context *c2, GP_Coord x2, GP_Coord y2)\n"
    "{\n"
    "	if (unlikely(w == 0 || h == 0)) return;\n\n"
    "	/* Special case - copy whole line-block with one memcpy() */\n"
    "	if ((x1 == 0) && (x2 == 0) && (w == c1->w) && (c1->w == c2->w) &&\n"
    "		(c1->bytes_per_row == c2->bytes_per_row)) {\n"
    "		memcpy(c2->pixels + c2->bytes_per_row * y2,\n"
    "		       c1->pixels + c1->bytes_per_row * y1,\n"
    "		       c1->bytes_per_row * h);\n"
    "		return;\n"
    "	}\n\n"
    "{% if size>=8 %}"
    "	/* General case - memcpy() each horizontal line */\n"
    "	for (GP_Size i = 0; i < h; i++)\n"
    "		memcpy(GP_PIXEL_ADDR_{{ size_suffix }}(c2, x2, y2 + i), \n"
    "		       GP_PIXEL_ADDR_{{ size_suffix }}(c1, x2, y2 + i),\n"
    "		       {{ size/8 }} * w);\n"
    "{% else %}" # subtle - rectangles may not be byte aligned in the same way
    "	/* Alignment (index) of first bits in the first byte */\n"
    "	int al1 = GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x1);\n"
    "	int al2 = GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x2);\n"
    "	/* Special case of the same alignment and width >=2 bytes */\n"
    "	if ((al1 == al2) && (w * {{ size }} >= 16)) {\n"
    "		/* Number of bits in the last partial byte */\n"
    "		int end_al = GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x1 + w);\n"
    "		GP_ASSERT(({{ size }} * w - al1 - end_al) % 8 == 0);\n"
    "		int copy_size = ({{ size }} * w - al1 - end_al) / 8;\n"
    "		/* First and last byte incident to the line */\n"
    "		uint8_t *p1 = GP_PIXEL_ADDR_{{ size_suffix }}(c1, x1, y1);\n"
    "		uint8_t *p2 = GP_PIXEL_ADDR_{{ size_suffix }}(c2, x2, y2);\n"
    "		uint8_t *end_p1 = GP_PIXEL_ADDR_{{ size_suffix }}(c1, x1 + w - 1, y1);\n"
    "		uint8_t *end_p2 = GP_PIXEL_ADDR_{{ size_suffix }}(c2, x2 + w - 1, y2);\n"
    "		for (GP_Size i = 0; i < h; i++) {\n"
    "			if (al1 != 0)\n"
    "				GP_SET_BITS(al1, 8-al1, *p2, GP_GET_BITS(al1, 8-al1, *p1));\n"
    "			memcpy(p2+(al1!=0), p1+(al1!=0), copy_size);\n"
    "			if (end_al != 0)\n"
    "				GP_SET_BITS(0, end_al, *end_p2, GP_GET_BITS(0, end_al, *end_p1));\n"
    "			p1 += c1->bytes_per_row;\n"
    "			end_p1 += c1->bytes_per_row;\n"
    "			p2 += c2->bytes_per_row;\n"
    "			end_p2 += c2->bytes_per_row;\n"
    "		}\n"
    "	} else /* Different bit-alignment, can't use memcpy() */\n"
    "		GP_Blit_Naive(c1, x1, y1, w, h, c2, x2, y2);\n"
    "{% endif %}"
    "}\n", size=size, size_suffix=size_suffix)

