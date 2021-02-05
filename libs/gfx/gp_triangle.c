// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/gp_pixmap.h"
#include <core/gp_transform.h>

#include <gfx/gp_line.h>
#include <gfx/gp_polygon.h>
#include <gfx/gp_triangle.h>

void gp_triangle_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                     gp_coord x1, gp_coord y1,
                     gp_coord x2, gp_coord y2, gp_pixel pixel)
{
	gp_line_raw(pixmap, x0, y0, x1, y1, pixel);
	gp_line_raw(pixmap, x0, y0, x2, y2, pixel);
	gp_line_raw(pixmap, x1, y1, x2, y2, pixel);
}

void gp_triangle(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                 gp_coord x1, gp_coord y1,
                 gp_coord x2, gp_coord y2, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);
	GP_TRANSFORM_POINT(pixmap, x2, y2);

	gp_triangle_raw(pixmap, x0, y0, x1, y1, x2, y2, pixel);
}

void gp_fill_triangle(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                     gp_coord x1, gp_coord y1,
                     gp_coord x2, gp_coord y2, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);
	GP_TRANSFORM_POINT(pixmap, x2, y2);

	gp_fill_triangle_raw(pixmap, x0, y0, x1, y1, x2, y2, pixel);
}
