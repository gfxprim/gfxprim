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

#ifndef GFX_GP_VLINE_H
#define GFX_GP_VLINE_H

#include "core/GP_Pixmap.h"

/* Raw per BPP HLines */
#include "gfx/GP_VLine.gen.h"

void GP_VLineXYY(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y0,
                 GP_Coord y1, GP_Pixel pixel);

void GP_VLineXYY_Raw(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y0,
                     GP_Coord y1, GP_Pixel pixel);

void GP_VLineXYH(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y, GP_Size h,
                 GP_Pixel pixel);

void GP_VLineXYH_Raw(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y, GP_Size h,
                     GP_Pixel pixel);

/* default argument set is XYY */
static inline void GP_VLine(GP_Pixmap *pixmap, GP_Coord x,
                            GP_Coord y0, GP_Coord y1, GP_Pixel pixel)
{
	GP_VLineXYY(pixmap, x, y0, y1, pixel);
}

static inline void GP_VLine_Raw(GP_Pixmap *pixmap, GP_Coord x,
                                GP_Coord y0, GP_Coord y1, GP_Pixel pixel)
{
	GP_VLineXYY_Raw(pixmap, x, y0, y1, pixel);
}

#endif /* GFX_GP_VLINE_H */
