@ include source.t
/*
 * Vertical Line drawing algorithm.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <gfx/gp_vline.gen.h>

@ for ps in pixelsizes:
void gp_vline_raw_{{ ps.suffix }}(gp_pixmap *pixmap, gp_coord x,
			gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	int y;

	for (y = y0; y <= y1; y++)
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, pixel);
}

void gp_vline_raw_{{ ps.suffix }}_clip(gp_pixmap *pixmap, gp_coord x,
                                       gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	if (x < 0 || x >= (gp_coord) pixmap->w ||
	    y1 < 0 || y0 >= (gp_coord) pixmap->h)
		return;

	y0 = GP_MAX(y0, 0);
	y1 = GP_MIN(y1, (gp_coord) pixmap->h - 1);

	gp_vline_raw_{{ ps.suffix }}_clip(pixmap, x, y0, y1, pixel);
}
