%% extends "base.c.t"

{% block descr %}Specialized blit functions and macros.{% endblock %}

%% block body
#include <string.h>

#include "core/GP_Pixel.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_Context.h"
#include "core/GP_Blit.h"
#include "core/GP_Debug.h"
#include "core/GP_Convert.h"
#include "core/GP_Convert.gen.h"
#include "core/GP_Convert_Scale.gen.h"

/*
 * TODO: this is used for same pixel but different offset, could still be optimized
 */
static void blitXYXY_Naive_Raw(const GP_Context *src,
                        GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
                        GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
	GP_Coord x, y;

	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++) {
			GP_Pixel p = GP_GetPixel_Raw(src, x, y);

			if (src->pixel_type != dst->pixel_type)
				p = GP_ConvertContextPixel(p, src, dst);

			GP_PutPixel_Raw(dst, x2 + (x - x0), y2 + (y - y0), p);
		}

	}
}

%% for ps in pixelsizes
/*
 * Blit for equal pixel types {{ ps.suffix }}
 */
static void blitXYXY_Raw_{{ ps.suffix }}(const GP_Context *src,
	GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
	GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
%% if not ps.needs_bit_endian()
	/* memcpy() each horizontal line */
	GP_Coord y;

	for (y = 0; y <= (y1 - y0); y++)
		memcpy(GP_PIXEL_ADDR_{{ ps.suffix }}(dst, x2, y2 + y),
		       GP_PIXEL_ADDR_{{ ps.suffix }}(src, x0, y0 + y),
		       {{ int(ps.size/8) }} * (x1 - x0 + 1));
%% else
{#	/* Rectangles may not be bit-aligned in the same way! */
	/* Alignment (index) of first bits in the first byte */
	//TODO: This is wrong for subcontexts where the offset
	//      needs to be summed with context->offset and moduled
	int al1 = GP_PIXEL_ADDR_OFFSET_{{ ps.suffix }}(x0);
	int al2 = GP_PIXEL_ADDR_OFFSET_{{ ps.suffix }}(x2);
	/* Special case of the same alignment and width >=2 bytes */
	if ((al1 == al2) && ((x1 - x0 + 1) * {{ ps.size }} >= 16)) {
		/* Number of bits in the last partial byte */
		int end_al = GP_PIXEL_ADDR_OFFSET_{{ ps.suffix }}(x1);
		GP_ASSERT(({{ ps.size }} * (x1 - x0 + 1) - al1 - end_al) % 8 == 0);
		int copy_size = ({{ ps.size }} * (x1 - x0 + 1) - al1 - end_al) / 8;
		/* First and last byte incident to the line */
		uint8_t *p1 = (uint8_t *) GP_PIXEL_ADDR_{{ ps.suffix }}(src, x1, y1);
		uint8_t *p2 = (uint8_t *) GP_PIXEL_ADDR_{{ ps.suffix }}(dst, x2, y2);
		uint8_t *end_p1 = (uint8_t *) GP_PIXEL_ADDR_{{ ps.suffix }}(src, x1, y0);
		uint8_t *end_p2 = (uint8_t *) GP_PIXEL_ADDR_{{ ps.suffix }}(dst, x2, y2);

		GP_Coord i;

		for (i = 0; i < (y1 - y0 + 1); i++) {
			if (al1 != 0)
				GP_SET_BITS(al1, 8-al1, *p2, GP_GET_BITS(al1, 8-al1, *p1));
			memcpy(p2+(al1!=0), p1+(al1!=0), copy_size);
			if (end_al != 0)
				GP_SET_BITS(0, end_al, *end_p2, GP_GET_BITS(0, end_al, *end_p1));
			p1 += src->bytes_per_row;
			end_p1 += src->bytes_per_row;
			p2 += dst->bytes_per_row;
			end_p2 += dst->bytes_per_row;
		}
	} else /* Different bit-alignment, can't use memcpy() */
#}
		blitXYXY_Naive_Raw(src, x0, y0, x1, y1, dst, x2, y2);
%% endif
}

%% endfor

/*
 * Generate Blits, I know this is n^2 variants but the gain is in speed is
 * more than 50% and the size footprint for two for cycles is really small.
 */
%% for src in pixeltypes
%% if not src.is_unknown() and not src.is_palette()
%% for dst in pixeltypes
%% if not dst.is_unknown() and not dst.is_palette()
%% if dst.name != src.name
/*
 * Blits {{ src.name }} to {{ dst.name }}
 */
static void blitXYXY_Raw_{{ src.name }}_{{ dst.name }}(const GP_Context *src,
	GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
	GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
	GP_Coord x, y;

	for (y = y0; y <= y1; y++)
		for (x = x0; x <= x1; x++) {
			GP_Pixel p1, p2 = 0;
			p1 = GP_GetPixel_Raw_{{ src.pixelsize.suffix }}(src, x, y);
			GP_Pixel_{{ src.name }}_TO_RGB888(p1, p2);
			GP_Pixel_RGB888_TO_{{ dst.name }}(p2, p1);
			GP_PutPixel_Raw_{{ dst.pixelsize.suffix }}(dst, x2 + (x - x0), y2 + (y - y0), p1);
		}
}

%% endif
%% endif
%% endfor
%% endif
%% endfor

void GP_BlitXYXY_Raw_Fast(const GP_Context *src,
                          GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
                          GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
	/* Same pixel type, could be (mostly) optimized to memcpy() */
	if (src->pixel_type == dst->pixel_type) {
		GP_FN_PER_BPP(blitXYXY_Raw, src->bpp, src->bit_endian,
		              src, x0, y0, x1, y1, dst, x2, y2);
		return;
	}

	/* Specialized functions */
	switch (src->pixel_type) {
%% for src in pixeltypes
%% if not src.is_unknown() and not src.is_palette()
	case GP_PIXEL_{{ src.name }}:
		switch (dst->pixel_type) {
%% for dst in pixeltypes
%% if not dst.is_unknown() and not dst.is_palette()
%% if dst.name != src.name
		case GP_PIXEL_{{ dst.name }}:
			blitXYXY_Raw_{{ src.name }}_{{ dst.name }}(src, x0, y0, x1, y1, dst, x2, y2);
		break;
%% endif
%% endif
%% endfor
		default:
			GP_ABORT("Invalid destination pixel %s",
			         GP_PixelTypeName(dst->pixel_type));
		}
	break;
%% endif
%% endfor
	default:
		GP_ABORT("Invalid source pixel %s",
		         GP_PixelTypeName(src->pixel_type));
	}
}

/*
 * And the same for non-raw variants.
 */
%% for src in pixeltypes
%% if not src.is_unknown() and not src.is_palette()
%% for dst in pixeltypes
%% if not dst.is_unknown() and not dst.is_palette()
%% if dst.name != src.name
/*
 * Blits {{ src.name }} to {{ dst.name }}
 */
static void blitXYXY_{{ src.name }}_{{ dst.name }}(const GP_Context *src,
	GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
	GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
	GP_Coord x, y, xt, yt;

	for (y = y0; y <= y1; y++)
		for (x = x0; x <= x1; x++) {
			GP_Pixel p1, p2 = 0;
			xt = x; yt = y;
			GP_TRANSFORM_POINT(src, xt, yt);
			p1 = GP_GetPixel_Raw_{{ src.pixelsize.suffix }}(src, xt, yt);
			GP_Pixel_{{ src.name }}_TO_RGB888(p1, p2);
			GP_Pixel_RGB888_TO_{{ dst.name }}(p2, p1);
			xt = x2 + (x - x0);
			yt = y2 + (y - y0);
			GP_TRANSFORM_POINT(dst, xt, yt);
			GP_PutPixel_Raw_{{ dst.pixelsize.suffix }}(dst, xt, yt, p1);
		}
}

%% endif
%% endif
%% endfor
%% endif
%% endfor

/*
 * Same pixel type but with rotation.
 */
%% for ps in pixelsizes
/*
 * Blits for same pixel type and bpp {{ ps.suffix }}
 */
static void blitXYXY_{{ ps.suffix }}(const GP_Context *src,
	GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
	GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
	GP_Coord x, y, xt, yt;

	for (y = y0; y <= y1; y++)
		for (x = x0; x <= x1; x++) {
			GP_Pixel p;
			xt = x; yt = y;
			GP_TRANSFORM_POINT(src, xt, yt);
			p = GP_GetPixel_Raw_{{ ps.suffix }}(src, xt, yt);
			xt = x2 + (x - x0);
			yt = y2 + (y - y0);
			GP_TRANSFORM_POINT(dst, xt, yt);
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, xt, yt, p);
		}
}
%% endfor

void GP_BlitXYXY_Fast(const GP_Context *src,
                      GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
                      GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
	/* Same pixel type */
	if (src->pixel_type == dst->pixel_type) {
		GP_FN_PER_BPP(blitXYXY, src->bpp, src->bit_endian,
		              src, x0, y0, x1, y1, dst, x2, y2);
		return;
	}
	
	/* Specialized functions */
	switch (src->pixel_type) {
%% for src in pixeltypes
%% if not src.is_unknown() and not src.is_palette()
	case GP_PIXEL_{{ src.name }}:
		switch (dst->pixel_type) {
%% for dst in pixeltypes
%% if not dst.is_unknown() and not dst.is_palette()
%% if dst.name != src.name
		case GP_PIXEL_{{ dst.name }}:
			blitXYXY_{{ src.name }}_{{ dst.name }}(src, x0, y0, x1, y1, dst, x2, y2);
		break;
%% endif
%% endif
%% endfor
		default:
			GP_ABORT("Invalid destination pixel %s",
			         GP_PixelTypeName(dst->pixel_type));
		}
	break;
%% endif
%% endfor
	default:
		GP_ABORT("Invalid source pixel %s",
		         GP_PixelTypeName(src->pixel_type));
	}
}

%% endblock body
