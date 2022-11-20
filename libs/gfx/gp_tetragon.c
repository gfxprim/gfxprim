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
#include <gfx/gp_tetragon.h>

void gp_tetragon_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                     gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
                     gp_coord x3, gp_coord y3, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	gp_line_raw(pixmap, x0, y0, x1, y1, pixel);
	gp_line_raw(pixmap, x1, y1, x2, y2, pixel);
	gp_line_raw(pixmap, x2, y2, x3, y3, pixel);
	gp_line_raw(pixmap, x3, y3, x0, y0, pixel);
}

void gp_tetragon(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                 gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
                 gp_coord x3, gp_coord y3, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);
	GP_TRANSFORM_POINT(pixmap, x2, y2);
	GP_TRANSFORM_POINT(pixmap, x3, y3);

	gp_tetragon_raw(pixmap, x0, y0, x1, y1, x2, y2, x3, y3, pixel);
}

void gp_fill_tetragon_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                          gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
                          gp_coord x3, gp_coord y3, gp_pixel pixel)
{
	const gp_coord xy[8] = {x0, y0, x1, y1, x2, y2, x3, y3};

	gp_fill_polygon_raw(pixmap, 0, 0, 4, xy, pixel);
}

void gp_fill_tetragon(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                      gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
                      gp_coord x3, gp_coord y3, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);
	GP_TRANSFORM_POINT(pixmap, x2, y2);
	GP_TRANSFORM_POINT(pixmap, x3, y3);

	const gp_coord xy[8] = {x0, y0, x1, y1, x2, y2, x3, y3};

	gp_fill_polygon_raw(pixmap, 0, 0, 4, xy, pixel);
}
