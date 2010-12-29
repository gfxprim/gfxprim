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
typedef struct GP_Context {
	void *pixels;			/* pointer to image pixels */
	uint8_t bits_per_pixel;		/* values: 1, 2, 4, 8, 16, 24, 32 */
	uint32_t bytes_per_row;
	uint32_t w;			/* width */
	uint32_t h;      		/* heigth */

	GP_PixelType pixel_type;        /* hardware pixel format */

	/* image orientation. Most common is landscape (0, 0, 0),
	 * portrait with normal topleft corner is (1, 0, 0).
	 */
	int axes_swap:1;	        /* swap axes so that x is y and y is x */
	int x_swap:1;		        /* swap direction on x  */
	int y_swap:1;		        /* swap direction on y  */

	/* clipping rectangle; drawing functions only affect the inside */
	uint32_t clip_w_min;
	uint32_t clip_w_max;
	uint32_t clip_h_min;
	uint32_t clip_h_max;
} GP_Context;

/* Determines the address of a pixel within the context's image.
 * Rows and columns are specified in the image's orientation
 * (i.e. they might not be XY if the image is rotated).
 */
#define GP_PIXEL_ADDRESS(context, w, h) ((uint8_t *) context->pixels \
	+ w * context->bytes_per_row \
	+ h * (context->bits_per_pixel / 8))

/* Evaluates to true if the context is valid (sane), false otherwise. */
#define GP_IS_CONTEXT_VALID(context) ( \
		context->w > 0 && context->h > 0 \
		&& context->clip_w_min <= context->clip_w_max \
		&& context->clip_w_max < context->w \
		&& context->clip_h_min <= context->clip_h_max \
		&& context->clip_h_max < context->h \
	)

/* Performs a series of sanity checks on context, aborting if any fails. */
#define GP_CHECK_CONTEXT(context) do { \
		GP_CHECK(context != NULL); \
		GP_CHECK(context->w > 0 && context->h > 0); \
		GP_CHECK(context->clip_w_min <= context->clip_w_max); \
		GP_CHECK(context->clip_h_min <= context->clip_h_max); \
		GP_CHECK(context->clip_w_max < context->w); \
		GP_CHECK(context->clip_h_max < context->h); \
	} while(0)

#endif /* GP_CONTEXT_H */
