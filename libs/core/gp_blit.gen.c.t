@ include source.t
/*
 * Specialized blit functions and macros.
 *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2011-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>

#include <core/gp_pixel.h>
#include <core/gp_get_put_pixel.h>
#include "core/gp_pixmap.h"
#include <core/gp_blit.h>
#include <core/gp_debug.h>
#include <core/gp_convert.h>
#include <core/gp_convert.gen.h>
#include <core/gp_convert_scale.gen.h>
#include <core/gp_mix_pixels2.gen.h>

/*
 * TODO: this is used for same pixel but different offset, could still be optimized
 */
static void blit_xyxy_naive_raw(const gp_pixmap *src,
                                gp_coord x0, gp_coord y0,
				gp_coord x1, gp_coord y1,
                                gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
	gp_coord x, y;

	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++) {
			gp_pixel p = gp_getpixel_raw(src, x, y);

			if (src->pixel_type != dst->pixel_type)
				p = gp_convert_pixmap_pixel(p, src, dst);

			gp_putpixel_raw(dst, x2 + (x - x0), y2 + (y - y0), p);
		}

	}
}

@ for ps in pixelpacks:
/*
 * Blit for equal pixel types {{ ps.suffix }}
 */
static void blitXYXY_Raw_{{ ps.suffix }}(const gp_pixmap *src,
	gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
	gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
@     if not ps.needs_bit_order():
	/* memcpy() each horizontal line */
	gp_coord y;

	for (y = 0; y <= (y1 - y0); y++)
		memcpy(GP_PIXEL_ADDR_{{ ps.suffix }}(dst, x2, y2 + y),
		       GP_PIXEL_ADDR_{{ ps.suffix }}(src, x0, y0 + y),
		       {{ int(ps.size/8) }} * (x1 - x0 + 1));
@     else:
# if 0
	/* Rectangles may not be bit-aligned in the same way! */
	/* Alignment (index) of first bits in the first byte */
	//TODO: This is wrong for subpixmaps where the offset
	//      needs to be summed with pixmap->offset and moduled
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

		gp_coord i;

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
#endif
		blit_xyxy_naive_raw(src, x0, y0, x1, y1, dst, x2, y2);
@     end
}

@ end
/*
 * Generate Blits, I know this is n^2 variants but the gain is in speed is
 * more than 50% and the size footprint for two for cycles is really small.
 */
@ for src in pixeltypes:
@     if not src.is_unknown() and not src.is_palette():
@         for dst in pixeltypes:
@             if not dst.is_unknown() and not dst.is_palette():
@                 if dst.name != src.name:
/*
 * Blits {{ src.name }} to {{ dst.name }}
 */
static void blitXYXY_Raw_{{ src.name }}_{{ dst.name }}(const gp_pixmap *src,
	gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
	gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
	gp_coord x, y, dx, dy;

	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++) {
			dx = x2 + (x - x0);
			dy = y2 + (y - y0);

			gp_pixel p1, p2 = 0, p3 = 0;

			p1 = gp_getpixel_raw_{{ src.pixelpack.suffix }}(src, x, y);
@                     if src.is_alpha():
			p2 = gp_getpixel_raw_{{ dst.pixelpack.suffix }}(dst, dx, dy);
			p3 = gp_mix_pixels_{{ src.name }}_{{ dst.name }}(p1, p2);
@                     else:
			GP_PIXEL_{{ src.name }}_TO_RGB888(p1, p2);
			GP_PIXEL_RGB888_TO_{{ dst.name }}(p2, p3);
@                     end
			gp_putpixel_raw_{{ dst.pixelpack.suffix }}(dst, dx, dy, p3);
		}
	}
}

@ end

void gp_blit_xyxy_raw_fast(const gp_pixmap *src,
                           gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
                           gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
	/* Same pixel type, could be (mostly) optimized to memcpy() */
	if (src->pixel_type == dst->pixel_type) {
		GP_FN_PER_PACK_PIXMAP(blitXYXY_Raw, src,
		                      src, x0, y0, x1, y1, dst, x2, y2);
		return;
	}

	/* Specialized functions */
	switch (src->pixel_type) {
@ for src in pixeltypes:
@     if not src.is_unknown() and not src.is_palette():
	case GP_PIXEL_{{ src.name }}:
		switch (dst->pixel_type) {
@         for dst in pixeltypes:
@             if not dst.is_unknown() and not dst.is_palette():
@                 if dst.name != src.name:
		case GP_PIXEL_{{ dst.name }}:
			blitXYXY_Raw_{{ src.name }}_{{ dst.name }}(src, x0, y0, x1, y1, dst, x2, y2);
		break;
@         end
		default:
			GP_ABORT("Invalid destination pixel %s",
			         gp_pixel_type_name(dst->pixel_type));
		}
	break;
@ end
	default:
		GP_ABORT("Invalid source pixel %s",
		         gp_pixel_type_name(src->pixel_type));
	}
}

/*
 * And the same for non-raw variants.
 */
@ for src in pixeltypes:
@     if not src.is_unknown() and not src.is_palette():
@         for dst in pixeltypes:
@             if not dst.is_unknown() and not dst.is_palette():
@                 if dst.name != src.name:
/*
 * Blits {{ src.name }} to {{ dst.name }}
 */
static void blitXYXY_{{ src.name }}_{{ dst.name }}(const gp_pixmap *src,
	gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
	gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
	gp_coord x, y, xt, yt;

	for (y = y0; y <= y1; y++)
		for (x = x0; x <= x1; x++) {
			gp_pixel p1, p2 = 0;
			xt = x; yt = y;
			GP_TRANSFORM_POINT(src, xt, yt);
			p1 = gp_getpixel_raw_{{ src.pixelpack.suffix }}(src, xt, yt);
			GP_PIXEL_{{ src.name }}_TO_RGB888(p1, p2);
			GP_PIXEL_RGB888_TO_{{ dst.name }}(p2, p1);
			xt = x2 + (x - x0);
			yt = y2 + (y - y0);
			GP_TRANSFORM_POINT(dst, xt, yt);
			gp_putpixel_raw_{{ dst.pixelpack.suffix }}(dst, xt, yt, p1);
		}
}

@ end

/*
 * Same pixel type but with rotation.
 */
@ for ps in pixelpacks:
/*
 * Blits for same pixel type and bpp {{ ps.suffix }}
 */
static void blitXYXY_{{ ps.suffix }}(const gp_pixmap *src,
	gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
	gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
	gp_coord x, y, xt, yt;

	for (y = y0; y <= y1; y++)
		for (x = x0; x <= x1; x++) {
			gp_pixel p;
			xt = x; yt = y;
			GP_TRANSFORM_POINT(src, xt, yt);
			p = gp_getpixel_raw_{{ ps.suffix }}(src, xt, yt);
			xt = x2 + (x - x0);
			yt = y2 + (y - y0);
			GP_TRANSFORM_POINT(dst, xt, yt);
			gp_putpixel_raw_{{ ps.suffix }}(dst, xt, yt, p);
		}
}
@ end

void gp_blit_xyxy_fast(const gp_pixmap *src,
                       gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
                       gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
	GP_DEBUG(2, "Blitting %s -> %s",
	         gp_pixel_type_name(src->pixel_type),
	         gp_pixel_type_name(dst->pixel_type));

	/* Same pixel type */
	if (src->pixel_type == dst->pixel_type) {
		GP_FN_PER_PACK_PIXMAP(blitXYXY, src,
		                      src, x0, y0, x1, y1, dst, x2, y2);
		return;
	}

	if (gp_pixmap_rotation_equal(src, dst)) {
		gp_blit_xyxy_raw_fast(src, x0, y0, x1, y1, dst, x2, y2);
		return;
	}

	/* Specialized functions */
	switch (src->pixel_type) {
@ for src in pixeltypes:
@     if not src.is_unknown() and not src.is_palette():
	case GP_PIXEL_{{ src.name }}:
		switch (dst->pixel_type) {
@         for dst in pixeltypes:
@             if not dst.is_unknown() and not dst.is_palette():
@                 if dst.name != src.name:
		case GP_PIXEL_{{ dst.name }}:
			blitXYXY_{{ src.name }}_{{ dst.name }}(src, x0, y0, x1, y1, dst, x2, y2);
		break;
@         end
		default:
			GP_ABORT("Invalid destination pixel %s",
			         gp_pixel_type_name(dst->pixel_type));
		}
	break;
@ end
	default:
		GP_ABORT("Invalid source pixel %s",
		         gp_pixel_type_name(src->pixel_type));
	}
}
