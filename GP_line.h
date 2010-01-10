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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef GP_LINE_H
#define GP_LINE_H

#include "GP_backend.h"

#define TARGET_AND_COLOR GP_TARGET_TYPE *target, GP_COLOR_TYPE color

/*
 * Draws a line from (x0, y0) to (x1, y1), inclusive.
 * Coordinates are automatically clipped; overdrawing surface boundary
 * is safe.
 */
void GP_Line(TARGET_AND_COLOR, int x0, int y0, int x1, int y1);
void GP_Line_8bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1);
void GP_Line_16bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1);
void GP_Line_24bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1);
void GP_Line_32bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1);

/*
 * Optimized call for drawing horizontal lines.
 * A line is drawn from (x0, y) to (x1, y), inclusive.
 * Coordinates are automatically clipped; overdrawing surface boundary
 * is safe.
 */
void GP_HLine(TARGET_AND_COLOR, int x0, int x1, int y);
void GP_HLine_8bpp(TARGET_AND_COLOR, int x0, int x1, int y);
void GP_HLine_16bpp(TARGET_AND_COLOR, int x0, int x1, int y);
void GP_HLine_24bpp(TARGET_AND_COLOR, int x0, int x1, int y);
void GP_HLine_32bpp(TARGET_AND_COLOR, int x0, int x1, int y);

/*
 * Optimized call for drawing vertical lines.
 * A line is drawn from (x, y0) to (x, y1), inclusive.
 * Coordinates are automatically clipped; overdrawing surface boundary
 * is safe.
 */
void GP_VLine(TARGET_AND_COLOR, int x, int y0, int y1);
void GP_VLine_8bpp(TARGET_AND_COLOR, int x, int y0, int y1);
void GP_VLine_16bpp(TARGET_AND_COLOR, int x, int y0, int y1);
void GP_VLine_24bpp(TARGET_AND_COLOR, int x, int y0, int y1);
void GP_VLine_32bpp(TARGET_AND_COLOR, int x, int y0, int y1);

enum GP_LinePosition {
	GP_LINE_ABOVE,
	GP_LINE_BELOW,
	GP_LINE_CENTER, /* thickness is treated as radius */
};

void GP_HLineWide(TARGET_AND_COLOR, enum GP_LinePosition pos, uint8_t thickness, int x0, int x1, int y);
void GP_VLineWide(TARGET_AND_COLOR, enum GP_LinePosition pos, uint8_t thickness, int x, int y0, int y1);

#undef TARGET_AND_COLOR

#endif /* GP_LINE_H */

