%% extends "base.c.t"

{% block descr %}Specialized blit functions and macros.{% endblock %}

%% block body
#include <stdio.h>
#include <string.h>
#include "GP_Pixel.h"
#include "GP.h"
#include "GP_Context.h"
#include "GP_Blit.h"

%% for ps in pixelsizes
/*** Blit preservimg PixelType, variant for {{ ps.suffix }} ***/
void GP_Blit_{{ ps.suffix }}(const GP_Context *c1, GP_Coord x1, GP_Coord y1, GP_Size w, GP_Size h,
			GP_Context *c2, GP_Coord x2, GP_Coord y2)
{
	if (unlikely(w == 0 || h == 0)) return;

	/* Special case - copy whole line-block with one memcpy() */
	if ((x1 == 0) && (x2 == 0) && (w == c1->w) && (c1->w == c2->w) &&
		(c1->bytes_per_row == c2->bytes_per_row)) {
		memcpy(c2->pixels + c2->bytes_per_row * y2,
		       c1->pixels + c1->bytes_per_row * y1,
		       c1->bytes_per_row * h);
		return;
	}

%% if not ps.needs_bit_endian()
	/* General case - memcpy() each horizontal line */
	for (GP_Size i = 0; i < h; i++)
		memcpy(GP_PIXEL_ADDR_{{ ps.suffix }}(c2, x2, y2 + i),
		       GP_PIXEL_ADDR_{{ ps.suffix }}(c1, x2, y2 + i),
		       {{ ps.size/8 }} * w);
%% else
	/* Rectangles may not be bit-aligned in the same way! */
	/* Alignment (index) of first bits in the first byte */
	int al1 = GP_PIXEL_ADDR_OFFSET_{{ ps.suffix }}(x1);
	int al2 = GP_PIXEL_ADDR_OFFSET_{{ ps.suffix }}(x2);
	/* Special case of the same alignment and width >=2 bytes */
	if ((al1 == al2) && (w * {{ ps.size }} >= 16)) {
		/* Number of bits in the last partial byte */
		int end_al = GP_PIXEL_ADDR_OFFSET_{{ ps.suffix }}(x1 + w);
		GP_ASSERT(({{ ps.size }} * w - al1 - end_al) % 8 == 0);
		int copy_size = ({{ ps.size }} * w - al1 - end_al) / 8;
		/* First and last byte incident to the line */
		uint8_t *p1 = (uint8_t *) GP_PIXEL_ADDR_{{ ps.suffix }}(c1, x1, y1);
		uint8_t *p2 = (uint8_t *) GP_PIXEL_ADDR_{{ ps.suffix }}(c2, x2, y2);
		uint8_t *end_p1 = (uint8_t *) GP_PIXEL_ADDR_{{ ps.suffix }}(c1, x1 + w - 1, y1);
		uint8_t *end_p2 = (uint8_t *) GP_PIXEL_ADDR_{{ ps.suffix }}(c2, x2 + w - 1, y2);
		for (GP_Size i = 0; i < h; i++) {
			if (al1 != 0)
				GP_SET_BITS(al1, 8-al1, *p2, GP_GET_BITS(al1, 8-al1, *p1));
			memcpy(p2+(al1!=0), p1+(al1!=0), copy_size);
			if (end_al != 0)
				GP_SET_BITS(0, end_al, *end_p2, GP_GET_BITS(0, end_al, *end_p1));
			p1 += c1->bytes_per_row;
			end_p1 += c1->bytes_per_row;
			p2 += c2->bytes_per_row;
			end_p2 += c2->bytes_per_row;
		}
	} else /* Different bit-alignment, can't use memcpy() */
		GP_Blit_Naive(c1, x1, y1, w, h, c2, x2, y2);
%% endif
}


%% endfor
%% endblock body
