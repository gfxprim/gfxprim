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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 *                                                                           *
 *****************************************************************************/

#ifndef CORE_GP_TRANSFORM_H
#define CORE_GP_TRANSFORM_H

#include "GP_Common.h"
#include "GP_FixedPoint.h"

/*
 * Flip a coordinate within context according to context transformation.
 */
#define GP_TRANSFORM_X(context, x) do {   \
	if ((context)->x_swap)            \
		x = (context)->w - x - 1; \
} while (0)

#define GP_TRANSFORM_Y(context, y) do {   \
	if ((context)->y_swap)            \
		y = (context)->h - y - 1; \
} while (0)

/*
 * Fixed point variants.
 */
#define GP_TRANSFORM_X_FP(context, x) do {                \
	if ((context)->x_swap)                            \
		x = GP_FP_FROM_INT((context)->w - 1) - x; \
} while (0)

#define GP_TRANSFORM_Y_FP(context, y) do {                \
	if ((context)->y_swap)                            \
		y = GP_FP_FROM_INT((context)->h - 1) - y; \
} while (0)

/*
 * Swap coordinates (axes) according to context transformation.
 */
#define GP_TRANSFORM_SWAP(context, x, y) do { \
	if ((context)->axes_swap)             \
		GP_SWAP(x, y);                \
} while (0)

/*
 * Transform "user"-coordinates to "real"-coordinates according to context
 * transformation.
 */
#define GP_TRANSFORM_POINT(context, x, y) do { \
	GP_TRANSFORM_SWAP(context, x, y);      \
	GP_TRANSFORM_X(context, x);            \
	GP_TRANSFORM_Y(context, y);            \
} while (0)

#define GP_TRANSFORM_POINT_FP(context, x, y) do { \
	GP_TRANSFORM_SWAP(context, x, y);         \
	GP_TRANSFORM_X_FP(context, x);            \
	GP_TRANSFORM_Y_FP(context, y);            \
} while (0)

/*
 * Transform "user"-coordinates to "real"-coordinates of a rectangle corner
 * according to context transformation. Corner with min-coordinates is
 * transformed to (different) corner with min-coordinates etc.
 * Arguments x, y, w, h are modified.
 */
#define GP_TRANSFORM_RECT(context, x, y, w, h) do { \
	GP_TRANSFORM_SWAP(context, x, y);           \
	GP_TRANSFORM_SWAP(context, w, h);           \
	                                            \
	if ((context)->x_swap)                      \
		x = (context)->w - x - w;           \
	                                            \
	if ((context)->y_swap)                      \
		y = (context)->h - y - h;           \
} while (0)

#define GP_TRANSFORM_RECT_FP(context, x, y, w, h) do {    \
	GP_TRANSFORM_SWAP(context, x, y);                 \
	GP_TRANSFORM_SWAP(context, w, h);                 \
	                                                  \
	if ((context)->x_swap)                            \
		x = GP_FP_FROM_INT((context)->w) - x - w; \
	                                                  \
	if ((context)->y_swap)                            \
		y = GP_FP_FROM_INT((context)->h) - y - h; \
} while (0)

/*
 * Transform "user"-coordinates to "real"-coordinates for a blit
 * called as GP_Blit(c1, x1, y1, w, h, c2, x2, y2).
 * All x1, y1, x2, y2, w, h are adjusted.
 */
#define GP_TRANSFORM_BLIT(c1, x1, y1, w, h, c2, x2, y2) do { \
	GP_TRANSFORM_RECT(c1, x1, y1, w, h);            \
	int w2 = w, h2 = h;                             \
	GP_TRANSFORM_SWAP(c2, w2, h2);                  \
	GP_TRANSFORM_RECT(c2, x2, y2, w2, h2);          \
} while (0)

/*
 * Inverse transformation to GP_TRANSFORM_POINT.
 * Use for translating mouse pointer coordinates to coordinates on context.
 */
#define GP_RETRANSFORM_POINT(context, x, y) do { \
	GP_TRANSFORM_X(context, x);              \
	GP_TRANSFORM_Y(context, y);              \
	GP_TRANSFORM_SWAP(context, x, y);        \
} while (0)

#endif /* CORE_GP_TRANSFORM_H */
