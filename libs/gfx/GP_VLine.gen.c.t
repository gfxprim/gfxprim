@ include source.t
/*
 * Vertical Line drawing algorithm.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include "gfx/GP_VLine.gen.h"

@ for ps in pixelsizes:
void gp_vline_raw_{{ ps.suffix }}(gp_pixmap *pixmap, gp_coord x,
			gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	int y;

	for (y = y0; y <= y1; y++)
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, pixel);
}

