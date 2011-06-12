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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 *                                                                           *
 *****************************************************************************/

#ifndef CORE_GP_TESTINGCORE_H
#define CORE_GP_TESTINGCORE_H

#include "GP_Context.h"
#include "GP_Pixel.h"

/*
 * Return pixel value of random color of given type.
 * Uses GP_TestingRandom and is not really efficient.
 */
GP_Pixel GP_RandomColor(GP_PixelType type);

/* 
 * Set color of pixels in given bitap rectangle to random values.
 * Uses GP_TestingRandom and is not really efficient.
 */
void GP_RandomizeRect(GP_Context *context, GP_Coord x, GP_Coord y, GP_Size w, GP_Size h);

/*
 * Return true if the two colors are equal after conversion to RGBA8888.
 * Not very efficient.
 */
int GP_EqualColors(GP_Pixel p1, GP_PixelType t1, GP_Pixel p2, GP_PixelType t2);

/*
 * Compare two rectangles in two contexts. Return 1 on equal.
 * The colors are cmpared by first converting them to RGBA8888.
 * Somewhat inefficient.
 */
int GP_EqualRects(const GP_Context *c1, GP_Coord x1, GP_Coord y1, GP_Size w, GP_Size h,
                  const GP_Context *c2, GP_Coord x2, GP_Coord y2);

#endif /* CORE_GP_TESTINGCORE_H */
