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

void GP_RectXYXY(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                 GP_Coord x1, GP_Coord y1, GP_Pixel pixel);

void GP_RectXYXY_Raw(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                     GP_Coord x1, GP_Coord y1, GP_Pixel pixel);

void GP_RectXYWH(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y,
                 GP_Size w, GP_Size h, GP_Pixel pixel);

void GP_RectXYWH_Raw(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y,
                     GP_Size w, GP_Size h, GP_Pixel pixel);

/* The XYXY argument set is the default */
static inline void GP_Rect(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                           GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_RectXYXY(pixmap, x0, y0, x1, y1, pixel);
}

static inline void GP_Rect_Raw(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                               GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_RectXYXY_Raw(pixmap, x0, y0, x1, y1, pixel);
}

/* Filled Rectangle */

void GP_FillRectXYXY(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                     GP_Coord x1, GP_Coord y1, GP_Pixel pixel);

void GP_FillRectXYXY_Raw(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                         GP_Coord x1, GP_Coord y1, GP_Pixel pixel);

void GP_FillRectXYWH(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y,
                     GP_Size w, GP_Size h, GP_Pixel pixel);

void GP_FillRectXYWH_Raw(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y,
	                 GP_Size w, GP_Size h, GP_Pixel pixel);

/* The XYXY argument set is the default */
static inline void GP_FillRect(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                               GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_FillRectXYXY(pixmap, x0, y0, x1, y1, pixel);
}

static inline void GP_FillRect_Raw(GP_Pixmap *pixmap,
                                   GP_Coord x0, GP_Coord y0,
                                   GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_FillRectXYXY_Raw(pixmap, x0, y0, x1, y1, pixel);
}

#endif /* GP_RECT_H */
