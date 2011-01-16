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

#ifndef GP_TRANSFORM_H
#define GP_TRANSFORM_H

#define GP_TRANSFORM_X(context, x) do { \
	if ((context)->x_swap)          \
		x = (context)->w - x;   \
} while (0)

#define GP_TRANSFORM_Y(context, y) do { \
	if ((context)->y_swap)          \
		y = (context)->h - y;   \
} while (0)

#define GP_TRANSFORM_SWAP(context, x, y) do { \
	if ((context)->axes_swap)             \
		GP_SWAP(x, y);                \
} while (0)

#define GP_TRANSFORM_POINT(context, x, y) do { \
	GP_TRANSFORM_SWAP(context, x, y);      \
	GP_TRANSFORM_X(context, x);            \
	GP_TRANSFORM_Y(context, y);            \
} while (0)

#define GP_TRANSFORM_RECT(context, x, y, rw, rh) do { \
	GP_TRANSFORM_SWAP(context, x, y);             \
	GP_TRANSFORM_SWAP(context, w, h);             \
	if ((context)->x_swap) {                      \
		x = (context)->w - x - rw;            \
	}                                             \
	if ((context)->y_swap) {                      \
		y = (context)->h - y - rh;            \
	}                                             \
} while (0)

/*
 * Inverse transformation. Use for translating mouse pointer coordinates to
 * coordinates on context.
 */
#define GP_RETRANSFORM_POINT(context, x, y) do { \
	GP_TRANSFORM_X(context, x);              \
	GP_TRANSFORM_Y(context, y);              \
	GP_TRANSFORM_SWAP(context, x, y);        \
} while (0)

#endif /* GP_TRANSFORM_H */
