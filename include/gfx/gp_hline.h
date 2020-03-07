// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GFX_GP_HLINE_H
#define GFX_GP_HLINE_H

#include "core/gp_types.h"

/* Raw per BPP HLines */
#include <gfx/gp_hline.gen.h>

/* Generic HLines */
void gp_hline_xxy(gp_pixmap *pixmap, gp_coord x0, gp_coord x1, gp_coord y,
                  gp_pixel pixel);

void gp_hline_xxy_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord x1,
                      gp_coord y, gp_pixel pixel);

void gp_hline_xyw(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size w,
                  gp_pixel pixel);

void gp_hline_xyw_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size w,
                      gp_pixel pixel);

/* default argument set is xxy */
static inline void gp_hline_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord x1,
                                gp_coord y, gp_pixel p)
{
	gp_hline_xxy_raw(pixmap, x0, x1, y, p);
}

static inline void gp_hline(gp_pixmap *pixmap, gp_coord x0, gp_coord x1,
                            gp_coord y, gp_pixel p)
{
	gp_hline_xxy(pixmap, x0, x1, y, p);
}

#endif /* GFX_GP_HLINE_H */
