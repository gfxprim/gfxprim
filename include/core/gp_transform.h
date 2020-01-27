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

#include "core/gp_common.h"
#include <core/gp_fixed_point.h>

/*
 * Flip a coordinate within pixmap according to pixmap transformation.
 */
#define GP_TRANSFORM_X(pixmap, x) do {   \
	if ((pixmap)->x_swap)            \
		x = (pixmap)->w - x - 1; \
} while (0)

#define GP_TRANSFORM_Y(pixmap, y) do {   \
	if ((pixmap)->y_swap)            \
		y = (pixmap)->h - y - 1; \
} while (0)

/*
 * Fixed point variants.
 */
#define GP_TRANSFORM_X_FP(pixmap, x) do {                \
	if ((pixmap)->x_swap)                            \
		x = GP_FP_FROM_INT((pixmap)->w - 1) - x; \
} while (0)

#define GP_TRANSFORM_Y_FP(pixmap, y) do {                \
	if ((pixmap)->y_swap)                            \
		y = GP_FP_FROM_INT((pixmap)->h - 1) - y; \
} while (0)

/*
 * Swap coordinates (axes) according to pixmap transformation.
 */
#define GP_TRANSFORM_SWAP(pixmap, x, y) do { \
	if ((pixmap)->axes_swap)             \
		GP_SWAP(x, y);                \
} while (0)

/*
 * Transform "user"-coordinates to "real"-coordinates according to pixmap
 * transformation.
 */
#define GP_TRANSFORM_POINT(pixmap, x, y) do { \
	GP_TRANSFORM_SWAP(pixmap, x, y);      \
	GP_TRANSFORM_X(pixmap, x);            \
	GP_TRANSFORM_Y(pixmap, y);            \
} while (0)

#define GP_TRANSFORM_POINT_FP(pixmap, x, y) do { \
	GP_TRANSFORM_SWAP(pixmap, x, y);         \
	GP_TRANSFORM_X_FP(pixmap, x);            \
	GP_TRANSFORM_Y_FP(pixmap, y);            \
} while (0)

/*
 * Transform "user"-coordinates to "real"-coordinates of a rectangle corner
 * according to pixmap transformation. Corner with min-coordinates is
 * transformed to (different) corner with min-coordinates etc.
 * Arguments x, y, w, h are modified.
 */
#define GP_TRANSFORM_RECT(pixmap, x, y, w, h) do { \
	GP_TRANSFORM_SWAP(pixmap, x, y);           \
	GP_TRANSFORM_SWAP(pixmap, w, h);           \
	                                            \
	if ((pixmap)->x_swap)                      \
		x = (pixmap)->w - x - w;           \
	                                            \
	if ((pixmap)->y_swap)                      \
		y = (pixmap)->h - y - h;           \
} while (0)

#define GP_TRANSFORM_RECT_FP(pixmap, x, y, w, h) do {    \
	GP_TRANSFORM_SWAP(pixmap, x, y);                 \
	GP_TRANSFORM_SWAP(pixmap, w, h);                 \
	                                                  \
	if ((pixmap)->x_swap)                            \
		x = GP_FP_FROM_INT((pixmap)->w) - x - w; \
	                                                  \
	if ((pixmap)->y_swap)                            \
		y = GP_FP_FROM_INT((pixmap)->h) - y - h; \
} while (0)

/*
 * Transform "user"-coordinates to "real"-coordinates for a blit
 * called as gp_blit(c1, x1, y1, w, h, c2, x2, y2).
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
 * Use for translating mouse pointer coordinates to coordinates on pixmap.
 */
#define GP_RETRANSFORM_POINT(pixmap, x, y) do { \
	GP_TRANSFORM_X(pixmap, x);              \
	GP_TRANSFORM_Y(pixmap, y);              \
	GP_TRANSFORM_SWAP(pixmap, x, y);        \
} while (0)

#endif /* CORE_GP_TRANSFORM_H */
