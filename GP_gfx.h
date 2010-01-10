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

#ifndef GP_GFX_H
#define GP_GFX_H

#include "GP_backend.h"

/*
 * Routines for drawing various basic shapes.
 * All these calls honor the target clipping rectangle, and are safe
 * if the shape partly or completely lies outside the target buffer.
 */

/*
 * First two arguments of all these calls are the same:
 * the target buffer and color of the shape.
 */
#define TARGET_AND_COLOR GP_TARGET_TYPE *target, GP_COLOR_TYPE color

/*
 * Clears the whole clipping rectangle to the specified color.
 */
void GP_Clear(TARGET_AND_COLOR);

/*
 * Circle and filled circle. The coordinates specify the center point,
 * and the radius (in pixels, inclusive). The circle should always
 * fit into a rectangle (xcenter-r, ycenter-r, xcenter+r, ycenter+r).
 */

void GP_Circle(TARGET_AND_COLOR, int xcenter, int ycenter, int r);
void GP_FillCircle(TARGET_AND_COLOR, int xcenter, int ycenter, int r);

/*
 * Axis-aligned ellipses. The coordinates specify the center point
 * and two radii.
 */

void GP_Ellipse(TARGET_AND_COLOR, int xcenter, int ycenter, int a, int b);
void GP_FillEllipse(TARGET_AND_COLOR, int xcenter, int ycenter, int a, int b);

/*
 * Rectangle and filled rectangle. The coordinates are inclusive,
 * i.e. the rectangle spans over whole range <x0, x1> and <y0, y1>.
 */

void GP_Rect(TARGET_AND_COLOR, int x0, int y0, int x1, int y1);
void GP_FillRect(TARGET_AND_COLOR, int x0, int y0, int x1, int y1);

/*
 * Triangle and filled triangle. Coordinates are inclusive, i.e. all
 * three vertices are parts of the drawn shape.
 */

void GP_Triangle(TARGET_AND_COLOR, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_FillTriangle(TARGET_AND_COLOR, int x0, int y0, int x1, int y1, int x2, int y2);

/*
 * Following calls are specializations of the above functions
 * to various bit depths.
 */

void GP_Circle_8bpp(TARGET_AND_COLOR, int xcenter, int ycenter, int r);
void GP_Circle_16bpp(TARGET_AND_COLOR, int xcenter, int ycenter, int r);
void GP_Circle_24bpp(TARGET_AND_COLOR, int xcenter, int ycenter, int r);
void GP_Circle_32bpp(TARGET_AND_COLOR, int xcenter, int ycenter, int r);

void GP_Triangle_8bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_Triangle_16bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_Triangle_24bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_Triangle_32bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1, int x2, int y2);

void GP_FillTriangle_8bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_FillTriangle_16bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_FillTriangle_24bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_FillTriangle_32bpp(TARGET_AND_COLOR, int x0, int y0, int x1, int y1, int x2, int y2);

#undef TARGET_AND_COLOR

#endif /* GP_GFX_H */

