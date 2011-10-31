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

#ifndef CORE_GP_CONTEXT_H
#define CORE_GP_CONTEXT_H

#include <stdint.h>
#include <unistd.h>

#include "GP_Common.h"
#include "GP_Types.h"
#include "GP_Pixel.h"

/* This structure holds all information needed for drawing into an image. */
typedef struct GP_Context {
	uint8_t *pixels;	 /* pointer to image pixels */
	uint8_t bpp;		 /* pixel length in bits */
	uint32_t bytes_per_row;
	uint32_t w;		 /* width in pixels */
	uint32_t h;      	 /* height in pixels */
	/* 
	 * Row bit offset. The offset is ignored for byte aligned pixels.
	 * Basically it's used for non aligned pixels with combination
	 * with subcontextes.
	 */
	uint8_t offset;          

	GP_PixelType pixel_type; /* hardware pixel format */

	/* image orientation. Most common is landscape (0, 0, 0),
	 * portrait with normal topleft corner is (1, 0, 0).
	 */
	uint8_t axes_swap:1;	/* swap axes so that x is y and y is x */
	uint8_t x_swap:1;	/* swap direction on x  */
	uint8_t y_swap:1;	/* swap direction on y  */
	uint8_t bit_endian:1;	/* GP_BIT_ENDIAN */
	uint8_t free_pixels:1;  /* If set pixels are freed on GP_ContextFree */
} GP_Context;

/* Determines the address of a pixel within the context's image.
 * Rows and columns are specified in the image's orientation
 * (i.e. they might not be XY if the image is rotated).
 */
#define GP_PIXEL_ADDR(context, x, y) (((context)->pixels \
	+ y * (context)->bytes_per_row \
	+ (x * (context)->bpp) / 8))

#define GP_CALC_ROW_SIZE(pixel_type, width) \
	 ((GP_PixelSize(pixel_type) * width) / 8 + \
	!!((GP_PixelSize(pixel_type) * width) % 8))

/* Performs a series of sanity checks on context, aborting if any fails. */
#define GP_CHECK_CONTEXT(context) do { \
	GP_CHECK(context, "NULL passed as context"); \
	GP_CHECK(context->pixels, "invalid context: NULL image pointer"); \
	GP_CHECK(context->bpp <= 32, "invalid context: unsupported bits-per-pixel count"); \
	GP_CHECK(context->w > 0 && context->h > 0, "invalid context: invalid image size"); \
} while (0)

/*
 * Is true, when pixel is clipped out of context.
 */
#define GP_PIXEL_IS_CLIPPED(context, x, y) \
	((x) < 0 || x >= (typeof(x)) context->w \
	|| (y) < 0 || y >= (typeof(y)) context->h) \

/*
 * Allocate context.
 */
GP_Context *GP_ContextAlloc(GP_Size w, GP_Size h, GP_PixelType type);

/*
 * If passed the pixels are copied to newly created context, otherwise
 * the pixels are allocated but uninitalized.
 */
#define GP_COPY_WITH_PIXELS 1

/*
 * Copy context.
 */
GP_Context *GP_ContextCopy(const GP_Context *src, int flag);

/*
 * Create subcontext.
 *
 * If pointer to subcontext is NULL, new context is allocated
 * otherwise context pointed by subcontext pointer is initalized.
 * 
 * The free_pixels flag is set to 0 upon subcontext initalization so the
 * GP_ContextFree() would not call free() upon the subcontext->pixels pointer.
 */
GP_Context *GP_ContextSubContext(GP_Context *context, GP_Context *subcontext,
                                 GP_Coord x, GP_Coord y, GP_Size w, GP_Size h);

/*
 * Converts context to different pixel type.
 *
 * This is naive implementation that doesn't do any ditherings or error
 * diffusions.
 */
GP_Context *GP_ContextConvert(const GP_Context *context, GP_PixelType res_type);

/*
 * Free context.
 */
void GP_ContextFree(GP_Context *context);

/*
 * Dump context into file
 */
GP_RetCode GP_ContextDump(GP_Context *context, const char *path);

/*
 * Rotates context flags clock wise.
 */
void GP_ContextFlagsRotateCW(GP_Context *context);

/*
 * Rotates context flags counter clock wise.
 */
void GP_ContextFlagsRotateCCW(GP_Context *context);

/*
 * Returns context width and height taking the rotation flags into a account. 
 */
static inline GP_Size GP_ContextW(const GP_Context *context)
{
	if (context->axes_swap)
		return context->h;
	else
		return context->w;
}

static inline GP_Size GP_ContextH(const GP_Context *context)
{
	if (context->axes_swap)
		return context->w;
	else
		return context->h;
}

#endif /* CORE_GP_CONTEXT_H */
