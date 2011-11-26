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

#include "GP_Core.h"
#include "GP_Blit.h"

#include <string.h>

GP_Context *GP_ContextCopy(const GP_Context *src, int flag)
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
		return NULL;
	}

	new->pixels = pixels;

	if (flag)
		memcpy(pixels, src->pixels, src->bytes_per_row * src->h);

	new->bpp           = src->bpp;
	new->bytes_per_row = src->bytes_per_row;
	new->offset        = 0;

	new->w = src->w;
	new->h = src->h;

	new->pixel_type = src->pixel_type;
	new->bit_endian = src->bit_endian;

	/* rotation and mirroring */
	new->axes_swap = src->axes_swap;
	new->y_swap    = src->y_swap;
	new->x_swap    = src->x_swap;
	
	new->free_pixels = 1;

	return new;
}

GP_Context *GP_ContextAlloc(GP_Size w, GP_Size h, GP_PixelType type)
{
	GP_Context *context = malloc(sizeof(GP_Context));
	uint32_t bpp = GP_PixelSize(type);
	uint32_t bpr = (bpp * w) / 8 + !!((bpp * w) % 8);
	void *pixels;

	pixels = malloc(bpr * h);

	if (pixels == NULL || context == NULL) {
		free(pixels);
		free(context);
		return NULL;
	}

	context->pixels         = pixels;
	context->bpp            = bpp;
	context->bytes_per_row  = bpr;
	context->offset         = 0;

	context->w = w;
	context->h = h;

	context->pixel_type = type;
	#warning Hmm, bit endianity... Why isn't this settled by different pixel types?
	context->bit_endian = 0;
	
	/* rotation and mirroring */
	context->axes_swap = 0;
	context->y_swap    = 0;
	context->x_swap    = 0;
	
	context->free_pixels = 1;

	return context;
}

GP_Context *GP_ContextConvert(const GP_Context *src,
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

void GP_ContextFree(GP_Context *context)
{
	if (context == NULL)
		return;

	if (context->free_pixels)
		free(context->pixels);

	free(context);
}

GP_Context *GP_ContextSubContext(GP_Context *context, GP_Context *subcontext,
                                 GP_Coord x, GP_Coord y, GP_Size w, GP_Size h)
{
	GP_CHECK(context, "NULL context");
	GP_CHECK(context->w >= x + w, "Subcontext w out of original context.");
	GP_CHECK(context->h >= y + h, "Subcontext h out of original context.");
	
	GP_Context *ret = subcontext;

	if (ret == NULL) {
		ret = malloc(sizeof(GP_Context));

		if (ret == NULL)
			return NULL;
	}

	ret->bpp           = context->bpp;
	ret->bytes_per_row = context->bytes_per_row;
	ret->offset        = (context->offset +
	                      GP_PixelAddrOffset(x, context->pixel_type)) % 8;

	ret->w = w;
	ret->h = h;

	ret->pixel_type = context->pixel_type;
	ret->bit_endian = context->bit_endian;

	/* rotation and mirroring */
	ret->axes_swap = context->axes_swap;
	ret->y_swap    = context->y_swap;
	ret->x_swap    = context->x_swap;

	ret->pixels = GP_PIXEL_ADDR(context, x, y);

	ret->free_pixels = 0;

	return ret;
}

GP_RetCode GP_ContextDump(GP_Context *context, const char *path)
{
	FILE *f = fopen(path, "w");
	uint32_t x, y;

	if (f == NULL)
		return GP_EBADFILE;

	for (y = 0; y < context->h; y++) {
		for (x = 0; x < context->bytes_per_row; x++)
			fprintf(f, "0x%02x ", ((uint8_t *)context->pixels)
			                      [y * context->bytes_per_row + x]);
		fprintf(f, "\n");
	}

	fclose(f);
	return GP_ESUCCESS;
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
void GP_ContextFlagsRotateCW(GP_Context *context)
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

void GP_ContextFlagsRotateCCW(GP_Context *context)
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
