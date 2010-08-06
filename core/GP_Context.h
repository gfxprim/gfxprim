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

#ifndef GP_CONTEXT_H
#define GP_CONTEXT_H

#include <stdint.h>
#include <unistd.h>

#include "GP_Check.h"
#include "GP_Pixel.h"

/* This structure holds all information needed for drawing into an image. */
typedef struct {
	void *pixels;			/* pointer to image pixels */
	uint8_t bits_per_pixel;		/* values: 8, 16, 24, 32 */
	uint32_t bytes_per_row;
	uint32_t rows;			/* total number of rows */
	uint32_t columns;		/* total number of columns */

	GP_PixelType pixel_type;        /* hardware pixel format */

	/* image orientation. Most common is landscape (0, 0, 0),
	 * portrait with normal topleft corner is (1, 0, 0).
	 */
	int vertical_rows:1;		/* are rows in fact vertical? */
	int right_to_left:1;		/* is zero at the right? */
	int bottom_to_top:1;		/* is zero at the bottom? */

	/* clipping rectangle; drawing functions only affect the inside */
	uint32_t clip_row_min;
	uint32_t clip_row_max;
	uint32_t clip_column_min;
	uint32_t clip_column_max;
} GP_Context;

/* Determines the address of a pixel within the context's image.
 * Rows and columns are specified in the image's orientation
 * (i.e. they might not be XY if the image is rotated).
 */
#define GP_PIXEL_ADDRESS(context, row, column) ((uint8_t *) context->pixels \
	+ row * context->bytes_per_row \
	+ column * (context->bits_per_pixel / 8))

/* Performs a series of sanity checks on context, aborting if any fails. */
#define GP_CHECK_CONTEXT(context) do { \
		GP_CHECK(context != NULL); \
		GP_CHECK(context->rows > 0 && context->columns > 0); \
		GP_CHECK(context->clip_row_min <= context->clip_row_max); \
		GP_CHECK(context->clip_column_min <= context->clip_column_max); \
		GP_CHECK(context->clip_row_max < context->rows); \
		GP_CHECK(context->clip_column_max < context->columns); \
	} while(0)

#endif /* GP_CONTEXT_H */
