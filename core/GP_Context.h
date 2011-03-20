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

#include "GP_Common.h"
#include "GP_Pixel.h"

/* This structure holds all information needed for drawing into an image. */
typedef struct GP_Context {
	uint8_t *pixels;	 /* pointer to image pixels */
	uint8_t bpp;		 /* values: 1, 2, 4, 8, 16, 24, 32 */
	uint32_t bytes_per_row;
	uint32_t w;		 /* width */
	uint32_t h;      	 /* heigth */

	GP_PixelType pixel_type; /* hardware pixel format */

	/* image orientation. Most common is landscape (0, 0, 0),
	 * portrait with normal topleft corner is (1, 0, 0).
	 */
	uint8_t axes_swap:1;	 	/* swap axes so that x is y and y is x */
	uint8_t x_swap:1;		/* swap direction on x  */
	uint8_t y_swap:1;		/* swap direction on y  */
	uint8_t bit_endian:1;		/* GP_BIT_ENDIAN */

	/* clipping rectangle; drawing functions only affect the inside */
	uint32_t clip_w_min;
	uint32_t clip_w_max;
	uint32_t clip_h_min;
	uint32_t clip_h_max;
} GP_Context;

/* Returns the pixel type used by the context. */
inline GP_PixelType GP_GetContextPixelType(const GP_Context *context);

/* Determines the address of a pixel within the context's image.
 * Rows and columns are specified in the image's orientation
 * (i.e. they might not be XY if the image is rotated).
 */
#define GP_PIXEL_ADDR(context, x, y) ((context->pixels \
	+ y * context->bytes_per_row \
	+ (x * context->bpp) / 8))

#define GP_CALC_ROW_SIZE(pixel_type, width) \
	 ((GP_PixelSize(pixel_type) * width) / 8 + \
	!!((GP_PixelSize(pixel_type) * width) % 8))

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
	} while (0)

/*
 * Is true, when pixel is clipped. 
 */
#define GP_PIXEL_IS_CLIPPED(context, x, y) \
	(x < (int) context->clip_w_min \
	|| x > (int) context->clip_w_max \
	|| y < (int) context->clip_h_min \
	|| y > (int) context->clip_h_max) \

/*
 * Allocate context.
 */
GP_Context *GP_ContextAlloc(uint32_t w, uint32_t h, GP_PixelType type);

/*
 * If passed the pixels are copied to newly created context, otherwise
 * the pixels are allocated but uninitalized.
 */
#define GP_COPY_WITH_PIXELS 1

/*
 * Copy context.
 */
GP_Context *GP_ContextCopy(GP_Context *context, int flag);

/*
 * Free context.
 */
void GP_ContextFree(GP_Context *context);

/*
 * Dump context into file
 */
GP_RetCode GP_ContextDump(GP_Context *context, const char *path);

#endif /* GP_CONTEXT_H */
