// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/gp_pixmap.h"
#include <core/gp_transform.h>

#include <gfx/gp_hline.h>
#include <gfx/gp_vline.h>
#include <gfx/gp_rect.h>

void gp_rect_xyxy_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                      gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_hline_raw(pixmap, x0, x1, y0, pixel);
	gp_hline_raw(pixmap, x0, x1, y1, pixel);
	gp_vline_raw(pixmap, x0, y0, y1, pixel);
	gp_vline_raw(pixmap, x1, y0, y1, pixel);
}

void gp_rect_xywh_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y,
	              gp_size w, gp_size h, gp_pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	gp_rect_xyxy_raw(pixmap, x, y, x + w - 1, y + h - 1, pixel);
}

void gp_rect_xyxy(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                  gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);

	gp_rect_xyxy_raw(pixmap, x0, y0, x1, y1, pixel);
}

void gp_rect_xywh(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                  gp_size w, gp_size h, gp_pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	gp_rect_xyxy(pixmap, x, y, x + w - 1, y + h - 1, pixel);
}

void gp_fill_rect_xyxy_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                           gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	if (y0 > y1)
		GP_SWAP(y0, y1);

	y0 = GP_MAX(0, y0);
	y1 = GP_MIN(y1, (gp_coord)pixmap->h - 1);

	gp_coord y;
	for (y = y0; y <= y1; y++)
		gp_hline_raw(pixmap, x0, x1, y, pixel);
}

void gp_fill_rect_xywh_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                           gp_size w, gp_size h, gp_pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	gp_fill_rect_xyxy_raw(pixmap, x, y, x + w - 1, y + h - 1, pixel);
}

void gp_fill_rect_xyxy(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                       gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);

	gp_fill_rect_xyxy_raw(pixmap, x0, y0, x1, y1, pixel);
}

void gp_fill_rect_xywh(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                       gp_size w, gp_size h, gp_pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	gp_fill_rect_xyxy(pixmap, x, y, x + w - 1, y + h - 1, pixel);
}
