// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GFX_GP_VLINE_H
#define GFX_GP_VLINE_H

#include "core/gp_types.h"

/* Raw per BPP HLines */
#include <gfx/gp_vline.gen.h>

void gp_vline_xyy(gp_pixmap *pixmap, gp_coord x, gp_coord y0,
                  gp_coord y1, gp_pixel pixel);

void gp_vline_xyy_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y0,
                      gp_coord y1, gp_pixel pixel);

void gp_vline_xyh(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size h,
                  gp_pixel pixel);

void gp_vline_xyh_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size h,
                      gp_pixel pixel);

/* default argument set is xyy */
static inline void gp_vline(gp_pixmap *pixmap, gp_coord x,
                            gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	gp_vline_xyy(pixmap, x, y0, y1, pixel);
}

static inline void gp_vline_raw(gp_pixmap *pixmap, gp_coord x,
                                gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	gp_vline_xyy_raw(pixmap, x, y0, y1, pixel);
}

#endif /* GFX_GP_VLINE_H */
