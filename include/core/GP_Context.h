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

#ifndef CORE_GP_CONTEXT_H
#define CORE_GP_CONTEXT_H

#include <stdint.h>
#include <unistd.h>

#include "GP_Common.h"
#include "GP_Types.h"
#include "GP_Pixel.h"

struct GP_Gamma;

/* This structure holds all information needed for drawing into an image. */
typedef struct GP_Context {
	uint8_t *pixels;	 /* pointer to image pixels */
	uint8_t bpp;		 /* pixel size in bits */
	uint32_t bytes_per_row;
	uint32_t w;		 /* width in pixels */
	uint32_t h;      	 /* height in pixels */
	/* 
	 * Row bit offset. The offset is ignored for byte aligned pixels.
	 * Basically it's used for non aligned pixels with combination
	 * with subcontextes.
	 */
	uint8_t offset;          

	/* 
	 * Pixel format. See GP_Pixel.gen.h and GP_Pixel.gen.c.
	 */
	enum GP_PixelType pixel_type;

	/*
	 * Pointer to optional Gamma table.
	 *
	 * If NULL, the channel values are considered linear.
	 *
	 * See GP_GammaCorrection.h.
	 */
	struct GP_Gamma *gamma;

	/* 
	 * Image orientation. Most common is landscape (0, 0, 0),
	 * portrait with normal topleft corner is (1, 0, 0).
	 */
	uint8_t axes_swap:1;	/* swap axes so that x is y and y is x */
	uint8_t x_swap:1;	/* swap direction on x */
	uint8_t y_swap:1;	/* swap direction on y */
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
 *
 * The context consists of two parts, the GP_Context structure and pixels array.
 *
 * The rotation flags are set to (0, 0, 0).
 */
GP_Context *GP_ContextAlloc(GP_Size w, GP_Size h, GP_PixelType type);

/*
 * Free context.
 *
 * If context->free_pixels, also free pixel data.
 */
void GP_ContextFree(GP_Context *context);

/*
 * Initalize context, pixels pointer is not dereferenced so it's safe to pass
 * NULL there and allocate it later with size context->bpr * context->h.
 *
 * The returned pointer is the pointer you passed as first argument.
 */
GP_Context *GP_ContextInit(GP_Context *context, GP_Size w, GP_Size h,
                           GP_PixelType type, void *pixels);

/*
 * Resizes context->pixels array and changes metadata to match the new size.
 *
 * Returns non-zero on failure (malloc() has failed).
 *
 * This call only resizes the pixel array. The pixel values, after resizing,
 * are __UNINITALIZED__ use resampling filters to resize image data.
 */
int GP_ContextResize(GP_Context *context, GP_Size w, GP_Size h);

enum GP_ContextCopyFlags {
	/* 
	 * Copy bitmap pixels too. If not set pixels are uninitalized.
	 */
	GP_COPY_WITH_PIXELS   = 0x01,
	/* 
	 * Copy image rotation flags. If not set flags are set to (0, 0, 0).
	 */
	GP_COPY_WITH_ROTATION = 0x02,
};

/*
 * Allocates a contex with exactly same values as source context.
 */
GP_Context *GP_ContextCopy(const GP_Context *src, int flags);

/*
 * Initalize subcontext. The returned pointer points to passed subcontext.
 */
GP_Context *GP_SubContext(const GP_Context *context, GP_Context *subcontext,
                          GP_Coord x, GP_Coord y, GP_Size w, GP_Size h);

/*
 * Allocate and initalize subcontext.
 *
 * The free_pixels flag is set to 0 upon subcontext initalization so the
 * GP_ContextFree() would not call free() upon the subcontext->pixels pointer.
 */
GP_Context *GP_SubContextAlloc(const GP_Context *context,
                               GP_Coord x, GP_Coord y, GP_Size w, GP_Size h);

/*
 * Converts context to a different pixel type.
 * Returns a newly allocated context.
 *
 * This is naive implementation that doesn't do any ditherings or error
 * diffusions.
 */
GP_Context *GP_ContextConvertAlloc(const GP_Context *src,
                                   GP_PixelType dst_pixel_type);

/*
 * Converts context to a different pixel type.
 *
 * This is naive implementation that doesn't do any ditherings or error
 * diffusions.
 */
GP_Context *GP_ContextConvert(const GP_Context *src, GP_Context *dst);

/*
 * Prints context information into stdout.
 */
void GP_ContextPrintInfo(const GP_Context *self);

/*
 * Rotates context flags clock wise.
 */
void GP_ContextRotateCW(GP_Context *context);

/*
 * Rotates context flags counter clock wise.
 */
void GP_ContextRotateCCW(GP_Context *context);

/*
 * Retruns 1 if rotation flags are equal.
 */
static inline int GP_ContextRotationEqual(const GP_Context *c1,
                                          const GP_Context *c2)
{
	return c1->axes_swap == c2->axes_swap &&
	       c1->x_swap == c2->x_swap &&
	       c1->y_swap == c2->y_swap;
}

/*
 * Sets rotation flags.
 */
static inline void GP_ContextSetRotation(GP_Context *dst, int axes_swap,
                                         int x_swap, int y_swap)
{
	dst->axes_swap = axes_swap;
	dst->x_swap = x_swap;
	dst->y_swap = y_swap;
}

/*
 * Copies rotation flags.
 */
static inline void GP_ContextCopyRotation(const GP_Context *src,
                                          GP_Context *dst)
{
	dst->axes_swap = src->axes_swap;
	dst->x_swap = src->x_swap;
	dst->y_swap = src->y_swap;
}

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

/*
 * Compare two contexts. Returns true only if all of types, sizes and
 * bitmap data match. Takes transformations into account.
 *
 * For now ignores gamma tables.
 *
 * Currently rather slow (getpixel).
 * TODO: speed up for same rotation and same bit-offset data (per-row memcpy).
 */

int GP_ContextEqual(const GP_Context *ctx1, const GP_Context *ctx2);

#endif /* CORE_GP_CONTEXT_H */

