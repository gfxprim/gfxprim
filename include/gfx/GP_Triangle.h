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

#ifndef GP_TRIANGLE_H
#define GP_TRIANGLE_H

#include "core/GP_Pixmap.h"

/* Triangle */

void GP_Triangle(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                 GP_Coord x1, GP_Coord y1,
                 GP_Coord x2, GP_Coord y2, GP_Pixel pixel);

void GP_Triangle_Raw(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                     GP_Coord x1, GP_Coord y1,
                     GP_Coord x2, GP_Coord y2, GP_Pixel pixel);

/* Filled Triangle */

void GP_FillTriangle(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                     GP_Coord x1, GP_Coord y1,
                     GP_Coord x2, GP_Coord y2, GP_Pixel pixel);

void GP_FillTriangle_Raw(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                         GP_Coord x1, GP_Coord y1,
                         GP_Coord x2, GP_Coord y2, GP_Pixel pixel);

#endif /* GP_TRIANGLE_H */
