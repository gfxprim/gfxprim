// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/gp_pixmap.h"
#include <core/gp_transform.h>
#include <core/gp_fn_per_bpp.h>

#include <gfx/gp_line_aa.h>

void gp_line_aa(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT_FP(pixmap, x0, y0);
	GP_TRANSFORM_POINT_FP(pixmap, x1, y1);

	gp_line_aa_raw(pixmap, x0, y0, x1, y1, pixel);
}
