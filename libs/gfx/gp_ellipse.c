// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_pixel_pack.gen.h>
#include <core/gp_transform.h>

#include <gfx/gp_ellipse.h>
#include <gfx/gp_hline.h>

#include "algo/Ellipse.algo.h"

/* Generate drawing functions for various bit depths. */
GP_DEF_DRAW_FN_PER_PACK(gp_ellipse_raw, DEF_ELLIPSE_FN)

void gp_ellipse_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                    gp_size a, gp_size b, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_PACK_PIXMAP(gp_ellipse_raw, pixmap, pixmap,
	                      xcenter, ycenter, a, b, pixel);
}

void gp_ellipse(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                gp_size a, gp_size b, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	/* recalculate center point and swap a and b when axes are swapped */
	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);
	GP_TRANSFORM_SWAP(pixmap, a, b);

	gp_ellipse_raw(pixmap, xcenter, ycenter, a, b, pixel);
}
