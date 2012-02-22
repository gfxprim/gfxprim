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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef GFX_GP_RECT_AA_H
#define GFX_GP_RECT_AA_H

#include "core/GP_Context.h"

/* Filled Rectangle */

void GP_FillRectXYXY_AA(GP_Context *context, GP_Coord x0, GP_Coord y0,
                        GP_Coord x1, GP_Coord y1, GP_Pixel pixel);

void GP_FillRectXYXY_AA_Raw(GP_Context *context, GP_Coord x0, GP_Coord y0,
                            GP_Coord x1, GP_Coord y1, GP_Pixel pixel);

void GP_FillRectXYWH_AA(GP_Context *context, GP_Coord x, GP_Coord y,
                        GP_Size w, GP_Size h, GP_Pixel pixel);

void GP_FillRectXYWH_AA_Raw(GP_Context *context, GP_Coord x, GP_Coord y,
	                    GP_Size w, GP_Size h, GP_Pixel pixel);

/* The XYXY argument set is the default */
static inline void GP_FillRect_AA(GP_Context *context, GP_Coord x0, GP_Coord y0,
                                  GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_FillRectXYXY_AA(context, x0, y0, x1, y1, pixel);
}

static inline void GP_FillRect_AA_Raw(GP_Context *context,
                                      GP_Coord x0, GP_Coord y0,
                                      GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_FillRectXYXY_AA_Raw(context, x0, y0, x1, y1, pixel);
}

#endif /* GFX_GP_RECT_AA_H */
