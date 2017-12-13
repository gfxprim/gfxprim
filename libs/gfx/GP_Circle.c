/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "core/GP_GetPutPixel.h"
#include "core/GP_FnPerBpp.h"

#include "gfx/GP_Circle.h"
#include "gfx/GP_HLine.h"

#include "algo/Circle.algo.h"

/* Generate drawing functions for various bit depths. */
GP_DEF_DRAW_FN_PER_BPP(gp_circle_raw, DEF_CIRCLE_FN)

void gp_circle_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                   gp_size r, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(gp_circle_raw, pixmap, pixmap,
	                      xcenter, ycenter, r, pixel);
}

void gp_circle(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
               gp_size r, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);

	gp_circle_raw(pixmap, xcenter, ycenter, r, pixel);
}

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
