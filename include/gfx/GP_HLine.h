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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef GFX_GP_HLINE_H
#define GFX_GP_HLINE_H

#include "core/gp_types.h"

/* Raw per BPP HLines */
#include "gfx/GP_HLine.gen.h"

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
