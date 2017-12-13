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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef GP_RECT_H
#define GP_RECT_H

#include "core/GP_Pixmap.h"

/* Rectangle */

void gp_rect_xyxy(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                  gp_coord x1, gp_coord y1, gp_pixel pixel);

void gp_rect_xyxy_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                      gp_coord x1, gp_coord y1, gp_pixel pixel);

void gp_rect_xywh(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                  gp_size w, gp_size h, gp_pixel pixel);

void gp_rect_xywh_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                      gp_size w, gp_size h, gp_pixel pixel);

/* The xyxy argument set is the default */
static inline void gp_rect(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                           gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_rect_xyxy(pixmap, x0, y0, x1, y1, pixel);
}

static inline void gp_rect_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                               gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_rect_xyxy_raw(pixmap, x0, y0, x1, y1, pixel);
}

/* Filled Rectangle */

void gp_fill_rect_xyxy(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                       gp_coord x1, gp_coord y1, gp_pixel pixel);

void gp_fill_rect_xyxy_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                           gp_coord x1, gp_coord y1, gp_pixel pixel);

void gp_fill_rect_xywh(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                       gp_size w, gp_size h, gp_pixel pixel);

void gp_fill_rect_xywh_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y,
	                   gp_size w, gp_size h, gp_pixel pixel);

/* The xyxy argument set is the default */
static inline void gp_fill_rect(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                                gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_fill_rect_xyxy(pixmap, x0, y0, x1, y1, pixel);
}

static inline void gp_fill_rect_raw(gp_pixmap *pixmap,
                                    gp_coord x0, gp_coord y0,
                                    gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_fill_rect_xyxy_raw(pixmap, x0, y0, x1, y1, pixel);
}

#endif /* GP_RECT_H */
