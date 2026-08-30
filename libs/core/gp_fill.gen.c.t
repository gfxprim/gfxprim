@ include source.t
@ include ../../include/core/write_pixels.t
/*
 * Optimized fill functions.
 *
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_pixmap.h>
#include <core/gp_write_pixels.gen.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_pixel_pack.gen.h>
#include <core/gp_pattern_fill.gen.h>
#include <core/gp_fill.h>

@ for ps in pixelpacks:
static void fill_{{ ps.suffix }}(gp_pixmap *pixmap, gp_pixel val)
{
	unsigned int y;
@     if ps.size <= 4:

	/* The tile is four rows high, so a patterned fill is four rows. */
	if (gp_pixel_is_pattern(val)) {
		gp_pixel_pattern_row row[GP_PIXEL_PATTERN_H];
		uint8_t off = GP_PIXEL_ADDR_OFFSET_{{ ps.suffix }}(pixmap, 0);
		unsigned int i;

		for (i = 0; i < GP_PIXEL_PATTERN_H; i++)
			gp_pixel_pattern_row_init_{{ ps.suffix }}(&row[i], pixmap, val, 0, i);

		for (y = 0; y < pixmap->h; y++) {
			gp_write_pixels_pattern_{{ ps.suffix }}(GP_PIXEL_ADDR_{{ ps.suffix }}(pixmap, 0, y),
			                                        off, pixmap->w,
			                                        &row[y % GP_PIXEL_PATTERN_H]);
		}

		return;
	}
@     end

	for (y = 0; y < pixmap->h; y++) {
@     if ps.suffix in optimized_writepixels:
		void *start = GP_PIXEL_ADDR(pixmap, 0, y);
@         if ps.needs_bit_order():
		gp_write_pixels_{{ ps.suffix }}(start, pixmap->offset, pixmap->w, val);
@         else:
		gp_write_pixels_{{ ps.suffix }}(start, pixmap->w, val);
@     else:
		unsigned int x;

		for (x = 0; x < pixmap->w; x++)
			gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, val);
@     end
	}
}

@ end
@
void gp_fill(gp_pixmap *pixmap, gp_pixel val)
{
	GP_FN_PER_PACK_PIXMAP(fill, pixmap, pixmap, val);
}
