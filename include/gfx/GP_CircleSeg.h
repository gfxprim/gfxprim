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

#ifndef GFX_GP_CIRCLE_H
#define GFX_GP_CIRCLE_H

#include "core/GP_Context.h"

/*
 * Quadrants in cartesian space the center is set in the middle of the circle.
 *
 * First segment is where both x and y are possitive, second is where only y is
 * possitive, third is for both x and y negative and the last one for only y
 * negative.
 *
 * Note that on computer screen (and in in-memory bitmaps) cordinates for y
 * grows in the opposite direction to the standard cartesian plane.
 *
 * So first segment is actually down right, second is down left, third is up
 * left, and fourth is up right.
 */
enum GP_CircleSegments {
	GP_CIRCLE_SEG1 = 0x01, /* First Quadrant  */
	GP_CIRCLE_SEG2 = 0x02, /* Second Quadrant */
	GP_CIRCLE_SEG3 = 0x04, /* Third Quadrant  */
	GP_CIRCLE_SEG4 = 0x08, /* Fourth Quadrant */
};

/* Circle Segment */

void GP_CircleSeg(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                  GP_Size r, uint8_t seg_flag, GP_Pixel pixel);

void GP_CircleSeg_Raw(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                      GP_Size r, uint8_t seg_flag, GP_Pixel pixel);

/* Filled Circle Segment */

void GP_FillCircleSeg(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                      GP_Size r, uint8_t seg_flag, GP_Pixel pixel);

void GP_FillCircleSeg_Raw(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                          GP_Size r, uint8_t seg_flag, GP_Pixel pixel);

#endif /* GFX_GP_CIRCLE_H */
