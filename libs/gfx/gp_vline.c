// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_common.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_pixel_pack.gen.h>
#include <core/gp_transform.h>

#include <gfx/gp_vline.h>
#include <gfx/gp_hline.h>

/*
 * Ensures that coordinates are in correct order, and clips them.
 * Exits immediately if the line is completely clipped out.
 */
#define ORDER_AND_CLIP_COORDS do {             \
	if (y0 > y1)                           \
		GP_SWAP(y0, y1);               \
	if (x < 0 || x >= (int) pixmap->w ||  \
	    y1 < 0 || y0 >= (int) pixmap->h)  \
		return;                        \
	y0 = GP_MAX(y0, 0);                    \
	y1 = GP_MIN(y1, (int) pixmap->h - 1); \
} while (0)

void gp_vline_xyy_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y0,
                      gp_coord y1, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	ORDER_AND_CLIP_COORDS;

	GP_FN_PER_PACK_PIXMAP(gp_vline_raw, pixmap, pixmap, x, y0, y1, pixel);
}

void gp_vline_xyh_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size h,
                     gp_pixel pixel)
{
	if (h == 0)
		return;

	gp_vline_xyy(pixmap, x, y, y + h - 1, pixel);
}

void gp_vline_xyy(gp_pixmap *pixmap, gp_coord x, gp_coord y0,
                  gp_coord y1, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	if (pixmap->axes_swap) {
		GP_TRANSFORM_Y(pixmap, x);
		GP_TRANSFORM_X(pixmap, y0);
		GP_TRANSFORM_X(pixmap, y1);
		gp_hline_raw(pixmap, y0, y1, x, pixel);
	} else {
		GP_TRANSFORM_X(pixmap, x);
		GP_TRANSFORM_Y(pixmap, y0);
		GP_TRANSFORM_Y(pixmap, y1);
		gp_vline_raw(pixmap, x, y0, y1, pixel);
	}
}

void gp_vline_xyh(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size h,
                 gp_pixel pixel)
{
	if (h == 0)
		return;

	gp_vline_xyy(pixmap, x, y, y + h - 1, pixel);
}
