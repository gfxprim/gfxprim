@ include source.t
/*
 * Horizontal line drawing
 *
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_GetPutPixel.gen.h"
#include "core/GP_WritePixel.h"

#include "gfx/GP_HLine.h"

@ # Explicit list of BPP that have optimized write pixel
@ have_writepixels = ['1BPP_LE', '1BPP_BE',
@                     '2BPP_LE', '2BPP_BE',
@                     '4BPP_LE', '4BPP_BE',
@                     '8BPP', '16BPP',
@                     '24BPP', '32BPP']
@
@ for ps in pixelsizes:
void gp_hline_raw_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int x1, int y,
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
	x1 = GP_MIN(x1, (int) pixmap->w - 1);

@     if ps.suffix in have_writepixels:
	size_t length = 1 + x1 - x0;
	void *start = GP_PIXEL_ADDR(pixmap, x0, y);

@         if ps.needs_bit_endian():
	unsigned int offset = GP_PIXEL_ADDR_OFFSET_{{ ps.suffix }}(x0);

	gp_write_pixels_{{ ps.suffix }}(start, offset, length, pixel);
@         else:
	gp_write_pixels_{{ ps.suffix }}(start, length, pixel);
@     else:
	for (;x0 <= x1; x0++)
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x0, y, pixel);
@     end
}

