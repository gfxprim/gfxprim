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

#ifndef GP_CIRCLE_H
#define GP_CIRCLE_H

#include "core/GP_Pixmap.h"

/* Circle */

void gp_circle(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
               gp_size r, gp_pixel pixel);

void gp_circle_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                   gp_size r, gp_pixel pixel);

/* Filled Circle */

void gp_fill_circle(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                    gp_size r, gp_pixel pixel);

void gp_fill_circle_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                        gp_size r, gp_pixel pixel);

/* Ring */

void gp_ring(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
             gp_size r1, gp_size r2, gp_pixel pixel);

void gp_ring_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                 gp_size r1, gp_size r2, gp_pixel pixel);

/* Filled Ring */

void gp_fill_ring(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                  gp_size r1, gp_size r2, gp_pixel pixel);

void gp_fill_ring_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                      gp_size r1, gp_size r2, gp_pixel pixel);

#endif /* GP_CIRCLE_H */
