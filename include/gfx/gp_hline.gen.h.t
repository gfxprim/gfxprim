@ include source.t
@ include ../../include/core/write_pixels.t
/*
 * Horizontal line drawing
 *
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GFX_GP_HLINE_GEN_H__
#define GFX_GP_HLINE_GEN_H__

#include <core/gp_get_put_pixel.gen.h>
#include <core/gp_write_pixels.gen.h>

@ for ps in pixelpacks:
static inline void gp_hline_raw_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int x1, int y,
			       gp_pixel pixel)
{
	/* draw always from left to right, swap coords if necessary */
	if (x0 > x1)
		GP_SWAP(x0, x1);

	/* return immediately if the line is completely out of surface */
	if (y < 0 || y >= (int) pixmap->h || x1 < 0 || x0 >= (int) pixmap->w)
		return;

	/* clip the line against surface boundaries */
	x0 = GP_MAX(x0, 0);
	x1 = GP_MIN(x1, (gp_coord) pixmap->w - 1);

@     if ps.suffix in optimized_writepixels:
	size_t length = 1 + x1 - x0;
	void *start = GP_PIXEL_ADDR_{{ ps.suffix }}(pixmap, x0, y);

@         if ps.needs_bit_order():
	unsigned int offset = GP_PIXEL_ADDR_OFFSET_{{ ps.suffix }}(pixmap, x0);
@             if ps.size == 1:
	gp_write_pixels_{{ ps.suffix }}(start, offset, y, length, pixel);
@             else:
	gp_write_pixels_{{ ps.suffix }}(start, offset, length, pixel);
@             end
@         else:
	gp_write_pixels_{{ ps.suffix }}(start, length, pixel);
@     else:
	for (;x0 <= x1; x0++)
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x0, y, pixel);
@     end
}

@ end
#endif /* GFX_GP_HLINE_GEN_H__ */
