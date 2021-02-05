// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_fn_per_bpp.h>

#include <gfx/gp_arc.h>

#include "algo/Arc.algo.h"

/* Generate drawing functions for various bit depths. */
GP_DEF_DRAW_FN_PER_BPP(gp_arc_segment_raw, DEF_ARCSEGMENT_FN)

void gp_arc_segment_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
		        gp_size a, gp_size b, int direction,
		        double start, double end,
		        gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(gp_arc_segment_raw, pixmap, pixmap,
	                     xcenter, ycenter, a, b, direction,
	                     start, end, pixel);
}

void gp_arc_segment(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
		    gp_size a, gp_size b, int direction,
		    double start, double end,
		    gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	/* recalculate center point and swap a and b when axes are swapped */
	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);
	GP_TRANSFORM_SWAP(pixmap, a, b);

	gp_arc_segment_raw(pixmap, xcenter, ycenter, a, b, direction,
			   start, end, pixel);
}
