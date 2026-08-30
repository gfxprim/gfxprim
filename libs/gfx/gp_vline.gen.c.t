@ include source.t
/*
 * Vertical Line drawing algorithm.
 *
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_pattern_fill.gen.h>
#include <gfx/gp_vline.gen.h>

@ for ps in pixelpacks:
void gp_vline_raw_{{ ps.suffix }}(gp_pixmap *pixmap, gp_coord x,
			gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	int y;
@     if ps.size > 4:
	for (y = y0; y <= y1; y++)
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, pixel);
@     else:
	if (!gp_pixel_is_pattern(pixel)) {
		for (y = y0; y <= y1; y++)
			gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, pixel);

		return;
	}

	gp_pixel_pattern_col col;

	if (y1 < y0)
		return;

	gp_pixel_pattern_col_init_{{ ps.suffix }}(&col, pixmap, pixel, x);
	gp_write_pixels_pattern_col(GP_PIXEL_ADDR_{{ ps.suffix }}(pixmap, x, y0),
	                            pixmap->bytes_per_row, y1 - y0 + 1, &col, y0);
@     end
}

void gp_vline_raw_{{ ps.suffix }}_clip(gp_pixmap *pixmap, gp_coord x,
                                       gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	if (x < 0 || x >= (gp_coord) pixmap->w ||
	    y1 < 0 || y0 >= (gp_coord) pixmap->h)
		return;

	y0 = GP_MAX(y0, 0);
	y1 = GP_MIN(y1, (gp_coord) pixmap->h - 1);

	gp_vline_raw_{{ ps.suffix }}(pixmap, x, y0, y1, pixel);
}
