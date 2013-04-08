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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <errno.h>
#include <string.h>

#include "GP_Debug.h"
#include "GP_Transform.h"
#include "GP_Pixel.h"
#include "GP_GetPutPixel.h"
#include "GP_Gamma.h"
#include "GP_Context.h"
#include "GP_Blit.h"

static uint32_t get_bpr(uint32_t bpp, uint32_t w)
{
	return (bpp * w) / 8 + !!((bpp * w) % 8);
}

GP_Context *GP_ContextAlloc(GP_Size w, GP_Size h, GP_PixelType type)
{
	GP_CHECK_VALID_PIXELTYPE(type);
	GP_Context *context;
	uint32_t bpp = GP_PixelSize(type);
	uint32_t bpr = get_bpr(bpp, w);
	void *pixels;

	GP_DEBUG(1, "Allocating context %u x %u - %s",
	         w, h, GP_PixelTypeName(type));

	if (w <= 0 || h <= 0) {
		GP_WARN("Trying to allocate context with zero width and/or height");
		errno = EINVAL;
		return NULL;
	}

	pixels = malloc(bpr * h);
	context = malloc(sizeof(GP_Context));

	if (pixels == NULL || context == NULL) {
		free(pixels);
		free(context);
		GP_WARN("Malloc failed :(");
		errno = ENOMEM;
		return NULL;
	}

	context->pixels        = pixels;
	context->bpp           = bpp;
	context->bytes_per_row = bpr;
	context->offset        = 0;

	context->w = w;
	context->h = h;

	context->gamma = NULL;

	context->pixel_type = type;
	#warning Hmm, bit endianity... Why is not this settled by different pixel types?
	context->bit_endian = GP_PixelTypes[type].bit_endian;

	/* rotation and mirroring */
	GP_ContextSetRotation(context, 0, 0, 0);

	context->free_pixels = 1;

	return context;
}

void GP_ContextFree(GP_Context *context)
{
	GP_DEBUG(1, "Freeing context (%p)", context);

	if (context == NULL)
		return;

	if (context->free_pixels)
		free(context->pixels);

	if (context->gamma)
		GP_GammaRelease(context->gamma);

	free(context);
}

GP_Context *GP_ContextInit(GP_Context *context, GP_Size w, GP_Size h,
                           GP_PixelType type, void *pixels)
{
	uint32_t bpp = GP_PixelSize(type);
	uint32_t bpr = get_bpr(bpp, w);

	context->pixels        = pixels;
	context->bpp           = bpp;
	context->bytes_per_row = bpr;
	context->offset        = 0;

	context->w = w;
	context->h = h;

	context->pixel_type = type;
	context->bit_endian = 0;
	
	context->gamma = NULL;

	/* rotation and mirroring */
	GP_ContextSetRotation(context, 0, 0, 0);

	context->free_pixels = 0;

	return context;
}

int GP_ContextResize(GP_Context *context, GP_Size w, GP_Size h)
{
	uint32_t bpr = get_bpr(context->bpp, w);
	void *pixels;

	pixels = realloc(context->pixels, bpr * h);

	if (pixels == NULL)
		return 1;

	context->w = w;
	context->h = h;
	context->bytes_per_row = bpr;
	context->pixels = pixels;

	return 0;
}

GP_Context *GP_ContextCopy(const GP_Context *src, int flags)
{
	GP_Context *new;
	uint8_t *pixels;

	if (src == NULL)
		return NULL;

	new     = malloc(sizeof(GP_Context));
	pixels  = malloc(src->bytes_per_row * src->h);

	if (pixels == NULL || new == NULL) {
		free(pixels);
		free(new);
		GP_WARN("Malloc failed :(");
		errno = ENOMEM;
		return NULL;
	}

	new->pixels = pixels;

	if (flags & GP_COPY_WITH_PIXELS)
		memcpy(pixels, src->pixels, src->bytes_per_row * src->h);

	new->bpp           = src->bpp;
	new->bytes_per_row = src->bytes_per_row;
	new->offset        = 0;

	new->w = src->w;
	new->h = src->h;

	new->pixel_type = src->pixel_type;
	new->bit_endian = src->bit_endian;

	if (flags & GP_COPY_WITH_ROTATION)
		GP_ContextCopyRotation(src, new);
	else
		GP_ContextSetRotation(new, 0, 0, 0);
	
	//TODO: Copy the gamma too
	new->gamma = NULL;
	
	new->free_pixels = 1;

	return new;
}


GP_Context *GP_ContextConvertAlloc(const GP_Context *src,
                                   GP_PixelType dst_pixel_type)
{
	int w = GP_ContextW(src);
	int h = GP_ContextH(src);

	GP_Context *ret = GP_ContextAlloc(w, h, dst_pixel_type);
	
	if (ret == NULL)
		return NULL;

	GP_Blit(src, 0, 0, w, h, ret, 0, 0);
	
	return ret;
}

GP_Context *GP_ContextConvert(const GP_Context *src, GP_Context *dst)
{
	//TODO: Asserts
	int w = GP_ContextW(src);
	int h = GP_ContextH(src);
	
	GP_Blit(src, 0, 0, w, h, dst, 0, 0);

	return dst;
}

GP_Context *GP_SubContextAlloc(const GP_Context *context,
                               GP_Coord x, GP_Coord y, GP_Size w, GP_Size h)
{
	GP_Context *res = malloc(sizeof(GP_Context));

	if (res == NULL) {
		GP_WARN("Malloc failed :(");
		errno = ENOMEM;
		return NULL;
	}

	return GP_SubContext(context, res, x, y, w, h);
}

GP_Context *GP_SubContext(const GP_Context *context, GP_Context *subcontext,
                          GP_Coord x, GP_Coord y, GP_Size w, GP_Size h)
{
	GP_CHECK(context, "NULL context");

	GP_TRANSFORM_RECT(context, x, y, w, h);

	GP_CHECK(context->w >= x + w, "Subcontext w out of original context.");
	GP_CHECK(context->h >= y + h, "Subcontext h out of original context.");
	
	subcontext->bpp           = context->bpp;
	subcontext->bytes_per_row = context->bytes_per_row;
	subcontext->offset        = (context->offset +
	                            GP_PixelAddrOffset(x, context->pixel_type)) % 8;

	subcontext->w = w;
	subcontext->h = h;

	subcontext->pixel_type = context->pixel_type;
	subcontext->bit_endian = context->bit_endian;
	
	/* gamma */
	subcontext->gamma = context->gamma;

	/* rotation and mirroring */
	GP_ContextCopyRotation(context, subcontext);

	subcontext->pixels = GP_PIXEL_ADDR(context, x, y);

	subcontext->free_pixels = 0;

	return subcontext;
}

void GP_ContextPrintInfo(const GP_Context *self)
{
	printf("Context info\n");
	printf("------------\n");
	printf("Size\t%ux%u\n", self->w, self->h);
	printf("BPP\t%u\n", self->bpp);
	printf("BPR\t%u\n", self->bytes_per_row);
	printf("Pixel\t%s (%u)\n", GP_PixelTypeName(self->pixel_type),
	       self->pixel_type);
	printf("Offset\t%u (only unaligned pixel types)\n", self->offset);
	printf("Flags\taxes_swap=%u x_swap=%u y_swap=%u free_pixels=%u\n",
	       self->axes_swap, self->x_swap, self->y_swap, self->free_pixels);
	
	if (self->gamma)
		GP_GammaPrint(self->gamma);
}

/*
 * The context rotations consists of two cyclic permutation groups that are
 * mirrored.
 *
 * The flags change as follows:
 *
 * One group:
 *
 * x_swap y_swap axes_swap
 *      0      0         0
 *      1      0         1
 *      1      1         0
 *      0      1         1
 *
 * And mirrored group:
 *
 * x_swap y_swap axes_swap
 *      0      0         1
 *      1      0         0
 *      1      1         1
 *      0      1         0
 *
 */
void GP_ContextRotateCW(GP_Context *context)
{
	context->axes_swap = !context->axes_swap;

	if (!context->x_swap && !context->y_swap) {
		context->x_swap = 1;
		return;
	}
	
	if (context->x_swap && !context->y_swap) {
		context->y_swap = 1;
		return;
	}
	
	if (context->x_swap && context->y_swap) {
		context->x_swap = 0;
		return;
	}

	context->y_swap  = 0;
}

void GP_ContextRotateCCW(GP_Context *context)
{
	context->axes_swap = !context->axes_swap;

	if (!context->x_swap && !context->y_swap) {
		context->y_swap = 1;
		return;
	}
	
	if (context->x_swap && !context->y_swap) {
		context->x_swap = 0;
		return;
	}
	
	if (context->x_swap && context->y_swap) {
		context->y_swap = 0;
		return;
	}

	context->x_swap  = 1;
}

int GP_ContextEqual(const GP_Context *ctx1, const GP_Context *ctx2)
{
	if (ctx1->pixel_type != ctx2->pixel_type)
		return 0;

	if (GP_ContextW(ctx1) != GP_ContextW(ctx2))
		return 0;

	if (GP_ContextH(ctx1) != GP_ContextH(ctx2))
		return 0;
	
	GP_Coord x, y, w = GP_ContextW(ctx1), h = GP_ContextH(ctx1);

	for (x = 0; x < w; x++)
		for (y = 0; y < h; y++)
			if (GP_GetPixel(ctx1, x, y) != GP_GetPixel(ctx2, x, y))
				return 0;

	return 1;
}

