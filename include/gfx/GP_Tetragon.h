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

#ifndef GP_TETRAGON_H
#define GP_TETRAGON_H

#include "core/GP_Pixmap.h"

/* Tetragon */

void gp_tetragon(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                 gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
                 gp_coord x3, gp_coord y3, gp_pixel pixel);

void gp_tetragon_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                     gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
                     gp_coord x3, gp_coord y3, gp_pixel pixel);

/* Filled Tetragon */

void gp_fill_tetragon(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                      gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
		      gp_coord x3, gp_coord y3, gp_pixel pixel);

void gp_fill_tetragon_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                          gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
                          gp_coord x3, gp_coord y3, gp_pixel pixel);

#endif /* GP_TETRAGON_H */
