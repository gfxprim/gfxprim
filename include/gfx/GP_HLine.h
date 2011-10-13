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

#ifndef GP_HLINE_H
#define GP_HLINE_H

#include "core/GP_Context.h"

/* Raw per BPP HLines */

void GP_HLine_Raw_1BPP_LE(GP_Context *context, GP_Coord x0, GP_Coord x1,
                          GP_Coord y, GP_Pixel pixel);

void GP_HLine_Raw_1BPP_BE(GP_Context *context, GP_Coord x0, GP_Coord x1,
                          GP_Coord y, GP_Pixel pixel);

void GP_HLine_Raw_2BPP_LE(GP_Context *context, GP_Coord x0, GP_Coord x1,
                          GP_Coord y, GP_Pixel pixel);

void GP_HLine_Raw_2BPP_BE(GP_Context *context, GP_Coord x0, GP_Coord x1,
                          GP_Coord y, GP_Pixel pixel);

void GP_HLine_Raw_4BPP_LE(GP_Context *context, GP_Coord x0, GP_Coord x1,
                          GP_Coord y, GP_Pixel pixel);

void GP_HLine_Raw_4BPP_BE(GP_Context *context, GP_Coord x0, GP_Coord x1,
                          GP_Coord y, GP_Pixel pixel);

void GP_HLine_Raw_8BPP(GP_Context *context, GP_Coord x0, GP_Coord x1,
                       GP_Coord y, GP_Pixel pixel);

void GP_HLine_Raw_18BPP_LE(GP_Context *context, GP_Coord x0, GP_Coord x1,
                           GP_Coord y, GP_Pixel pixel);

void GP_HLine_Raw_16BPP(GP_Context *context, GP_Coord x0, GP_Coord x1,
                        GP_Coord y, GP_Pixel pixel);

void GP_HLine_Raw_24BPP(GP_Context *context, GP_Coord x0, GP_Coord x1,
                        GP_Coord y, GP_Pixel pixel);

void GP_HLine_Raw_32BPP(GP_Context *context, GP_Coord x0, GP_Coord x1,
                        GP_Coord y, GP_Pixel pixel);

/* Generic HLines */

void GP_HLineXXY(GP_Context *context, GP_Coord x0, GP_Coord x1, GP_Coord y,
                 GP_Pixel pixel);

void GP_HLineXXY_Raw(GP_Context *context, GP_Coord x0, GP_Coord x1,
                     GP_Coord y, GP_Pixel pixel);

void GP_HLineXYW(GP_Context *context, GP_Coord x, GP_Coord y, GP_Size w,
                 GP_Pixel pixel);

void GP_HLineXYW_Raw(GP_Context *context, GP_Coord x, GP_Coord y, GP_Size w,
                     GP_Pixel pixel);

/* default argument set is XXY */
static inline void GP_HLine_Raw(GP_Context *context, GP_Coord x0, GP_Coord x1,
                                GP_Coord y, GP_Pixel p)
{
	GP_HLineXXY_Raw(context, x0, x1, y, p);
}

static inline void GP_HLine(GP_Context *context, GP_Coord x0, GP_Coord x1,
                            GP_Coord y, GP_Pixel p)
{
	GP_HLineXXY(context, x0, x1, y, p);
}

#endif /* GP_HLINE_H */
