@ include source.t
@ include ../../include/core/write_pixels.t
/*
 * Optimized fill functions.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_pixmap.h>
#include <core/gp_write_pixels.gen.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_pixel_pack.gen.h>
#include <core/gp_fill.h>

@ for ps in pixelpacks:
static void fill_{{ ps.suffix }}(gp_pixmap *pixmap, gp_pixel val)
{
	unsigned int y;

	for (y = 0; y < pixmap->h; y++) {
@     if ps.suffix in optimized_writepixels:
		void *start = GP_PIXEL_ADDR(pixmap, 0, y);
@         if ps.size == 1:
		gp_write_pixels_{{ ps.suffix }}(start, pixmap->offset, y%2, pixmap->w, val);
@         elif ps.needs_bit_order():
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
