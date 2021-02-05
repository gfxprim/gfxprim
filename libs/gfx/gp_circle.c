// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_fn_per_bpp.h>

#include <gfx/gp_circle.h>
#include <gfx/gp_hline.h>

void gp_ring_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                 gp_size r1, gp_size r2, gp_pixel pixel)
{
	gp_circle_raw(pixmap, xcenter, ycenter, r1, pixel);
	gp_circle_raw(pixmap, xcenter, ycenter, r2, pixel);
}

void gp_ring(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
             gp_size r1, gp_size r2, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);

	gp_ring_raw(pixmap, xcenter, ycenter, r1, r2, pixel);
}

#include "algo/FillRing.algo.h"

/* Generate drawing functions for various bit depths. */
GP_DEF_FILL_FN_PER_BPP(gp_fill_ring_raw, DEF_FILLRING_FN)

void gp_fill_ring_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                     gp_size r1, gp_size r2, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(gp_fill_ring_raw, pixmap, pixmap,
	                     xcenter, ycenter, r1, r2, pixel);
}

void gp_fill_ring(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                  gp_size r1, gp_size r2, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);

	gp_fill_ring_raw(pixmap, xcenter, ycenter, r1, r2, pixel);
}
