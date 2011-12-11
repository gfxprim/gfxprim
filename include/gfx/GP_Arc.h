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

#ifndef GP_ARC_H
#define GP_ARC_H

#include "core/GP_Context.h"

#include <math.h>

/*
 * Bits for quadrant mask used with GP_ArcSegment().
 * Quadrants are ordered using the standard mathematical order, i.e.
 * the top right quadrant (where x > 0 and y < 0) is the first (#0),
 * then the top left, then the bottom left, and then the bottom right.
 */
#define GP_QUADRANT_0		1
#define GP_QUADRANT_1		2
#define GP_QUADRANT_2		4
#define GP_QUADRANT_3		8
#define GP_QUADRANT_MINUSMINUS	GP_QUADRANT_2
#define GP_QUADRANT_PLUSMINUS	GP_QUADRANT_0
#define GP_QUADRANT_MINUSPLUS	GP_QUADRANT_1
#define GP_QUADRANT_PLUSPLUS	GP_QUADRANT_3
#define GP_QUADRANT_ALL		15

void GP_ArcSegment(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
		GP_Size a, GP_Size b, int quadrant_mask,
		double start, double end,
		GP_Pixel pixel);

void GP_ArcSegment_Raw(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
		GP_Size a, GP_Size b, int quadrant_mask,
		double start, double end,
		GP_Pixel pixel);

#endif /* GP_ARC_H */
