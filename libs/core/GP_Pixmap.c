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
#include "GP_Pixmap.h"
#include "GP_Blit.h"

static uint32_t get_bpr(uint32_t bpp, uint32_t w)
{
	uint64_t bits_per_row = (uint64_t)bpp * w;
	uint8_t padd = !!(bits_per_row % 8);

	if (bits_per_row / 8 + padd > UINT32_MAX) {
		GP_WARN("Pixmap too wide %u (overflow detected)", w);
		return 0;
	}

	return bits_per_row / 8 + padd;
}

GP_Pixmap *GP_PixmapAlloc(GP_Size w, GP_Size h, GP_PixelType type)
{
	GP_Pixmap *pixmap;
	uint32_t bpp;
	size_t bpr;
	void *pixels;

	if (!GP_VALID_PIXELTYPE(type)) {
		GP_WARN("Invalid pixel type %u", type);
		errno = EINVAL;
		return NULL;
	}

	if (w <= 0 || h <= 0) {
		GP_WARN("Trying to allocate pixmap with zero width and/or height");
		errno = EINVAL;
		return NULL;
	}

	GP_DEBUG(1, "Allocating pixmap %u x %u - %s",
	         w, h, GP_PixelTypeName(type));

	bpp = GP_PixelSize(type);

	if (!(bpr = get_bpr(bpp, w)))
		return NULL;

	size_t size = bpr * h;

	if (size / h != bpr) {
		GP_WARN("Pixmap too big %u x %u (owerflow detected)", w, h);
		return NULL;
	}

	pixels = malloc(size);
	pixmap = malloc(sizeof(GP_Pixmap));

	if (pixels == NULL || pixmap == NULL) {
		free(pixels);
		free(pixmap);
		GP_WARN("Malloc failed :(");
		errno = ENOMEM;
		return NULL;
	}

	pixmap->pixels        = pixels;
	pixmap->bpp           = bpp;
	pixmap->bytes_per_row = bpr;
	pixmap->offset        = 0;

	pixmap->w = w;
	pixmap->h = h;

	pixmap->gamma = NULL;

	pixmap->pixel_type = type;
	#warning Hmm, bit endianity... Why is not this settled by different pixel types?
	pixmap->bit_endian = GP_PixelTypes[type].bit_endian;

	/* rotation and mirroring */
	GP_PixmapSetRotation(pixmap, 0, 0, 0);

	pixmap->free_pixels = 1;

	return pixmap;
}

int GP_PixmapSetGamma(GP_Pixmap *self, float gamma)
{
	GP_GammaRelease(self->gamma);

	self->gamma = GP_GammaAcquire(self->pixel_type, gamma);

	return !self->gamma;
}

void GP_PixmapFree(GP_Pixmap *pixmap)
{
	GP_DEBUG(1, "Freeing pixmap (%p)", pixmap);

	if (pixmap == NULL)
		return;

	if (pixmap->free_pixels)
		free(pixmap->pixels);

	if (pixmap->gamma)
		GP_GammaRelease(pixmap->gamma);

	free(pixmap);
}

GP_Pixmap *GP_PixmapInit(GP_Pixmap *pixmap, GP_Size w, GP_Size h,
                           GP_PixelType type, void *pixels)
{
	uint32_t bpp = GP_PixelSize(type);
	uint32_t bpr = get_bpr(bpp, w);

	pixmap->pixels        = pixels;
	pixmap->bpp           = bpp;
	pixmap->bytes_per_row = bpr;
	pixmap->offset        = 0;

	pixmap->w = w;
	pixmap->h = h;

	pixmap->pixel_type = type;
	pixmap->bit_endian = 0;

	pixmap->gamma = NULL;

	/* rotation and mirroring */
	GP_PixmapSetRotation(pixmap, 0, 0, 0);

	pixmap->free_pixels = 0;

	return pixmap;
}

int GP_PixmapResize(GP_Pixmap *pixmap, GP_Size w, GP_Size h)
{
	uint32_t bpr = get_bpr(pixmap->bpp, w);
	void *pixels;

	pixels = realloc(pixmap->pixels, bpr * h);

	if (pixels == NULL)
		return 1;

	pixmap->w = w;
	pixmap->h = h;
	pixmap->bytes_per_row = bpr;
	pixmap->pixels = pixels;

	return 0;
}

GP_Pixmap *GP_PixmapCopy(const GP_Pixmap *src, int flags)
{
	GP_Pixmap *new;
	uint8_t *pixels;

	if (src == NULL)
		return NULL;

	new     = malloc(sizeof(GP_Pixmap));
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
		GP_PixmapCopyRotation(src, new);
	else
		GP_PixmapSetRotation(new, 0, 0, 0);

	//TODO: Copy the gamma too
	new->gamma = NULL;

	new->free_pixels = 1;

	return new;
}


GP_Pixmap *GP_PixmapConvertAlloc(const GP_Pixmap *src,
                                   GP_PixelType dst_pixel_type)
{
	int w = GP_PixmapW(src);
	int h = GP_PixmapH(src);

	GP_Pixmap *ret = GP_PixmapAlloc(w, h, dst_pixel_type);

	if (ret == NULL)
		return NULL;

	/*
	 * Fill the buffer with zeroes, otherwise it will
	 * contain random data which will generate mess
	 * when converting image with alpha channel.
	 */
	memset(ret->pixels, 0, ret->bytes_per_row * ret->h);

	GP_Blit(src, 0, 0, w, h, ret, 0, 0);

	return ret;
}

GP_Pixmap *GP_PixmapConvert(const GP_Pixmap *src, GP_Pixmap *dst)
{
	//TODO: Asserts
	int w = GP_PixmapW(src);
	int h = GP_PixmapH(src);

	/*
	 * Fill the buffer with zeroes, otherwise it will
	 * contain random data which will generate mess
	 * when converting image with alpha channel.
	 */
	memset(dst->pixels, 0, dst->bytes_per_row * dst->h);

	GP_Blit(src, 0, 0, w, h, dst, 0, 0);

	return dst;
}

GP_Pixmap *GP_SubPixmapAlloc(const GP_Pixmap *pixmap,
                               GP_Coord x, GP_Coord y, GP_Size w, GP_Size h)
{
	GP_Pixmap *res = malloc(sizeof(GP_Pixmap));

	if (res == NULL) {
		GP_WARN("Malloc failed :(");
		errno = ENOMEM;
		return NULL;
	}

	return GP_SubPixmap(pixmap, res, x, y, w, h);
}

GP_Pixmap *GP_SubPixmap(const GP_Pixmap *pixmap, GP_Pixmap *subpixmap,
                          GP_Coord x, GP_Coord y, GP_Size w, GP_Size h)
{
	GP_CHECK(pixmap, "NULL pixmap");

	GP_TRANSFORM_RECT(pixmap, x, y, w, h);

	GP_CHECK(pixmap->w >= x + w, "Subpixmap w out of original pixmap.");
	GP_CHECK(pixmap->h >= y + h, "Subpixmap h out of original pixmap.");

	subpixmap->bpp           = pixmap->bpp;
	subpixmap->bytes_per_row = pixmap->bytes_per_row;
	subpixmap->offset        = (pixmap->offset +
	                            GP_PixelAddrOffset(x, pixmap->pixel_type)) % 8;

	subpixmap->w = w;
	subpixmap->h = h;

	subpixmap->pixel_type = pixmap->pixel_type;
	subpixmap->bit_endian = pixmap->bit_endian;

	/* gamma */
	subpixmap->gamma = pixmap->gamma;

	/* rotation and mirroring */
	GP_PixmapCopyRotation(pixmap, subpixmap);

	subpixmap->pixels = GP_PIXEL_ADDR(pixmap, x, y);

	subpixmap->free_pixels = 0;

	return subpixmap;
}

void GP_PixmapPrintInfo(const GP_Pixmap *self)
{
	printf("Pixmap info\n");
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
 * The pixmap rotations consists of two cyclic permutation groups that are
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
void GP_PixmapRotateCW(GP_Pixmap *pixmap)
{
	pixmap->axes_swap = !pixmap->axes_swap;

	if (!pixmap->x_swap && !pixmap->y_swap) {
		pixmap->x_swap = 1;
		return;
	}

	if (pixmap->x_swap && !pixmap->y_swap) {
		pixmap->y_swap = 1;
		return;
	}

	if (pixmap->x_swap && pixmap->y_swap) {
		pixmap->x_swap = 0;
		return;
	}

	pixmap->y_swap  = 0;
}

void GP_PixmapRotateCCW(GP_Pixmap *pixmap)
{
	pixmap->axes_swap = !pixmap->axes_swap;

	if (!pixmap->x_swap && !pixmap->y_swap) {
		pixmap->y_swap = 1;
		return;
	}

	if (pixmap->x_swap && !pixmap->y_swap) {
		pixmap->x_swap = 0;
		return;
	}

	if (pixmap->x_swap && pixmap->y_swap) {
		pixmap->y_swap = 0;
		return;
	}

	pixmap->x_swap  = 1;
}

int GP_PixmapEqual(const GP_Pixmap *pixmap1, const GP_Pixmap *pixmap2)
{
	if (pixmap1->pixel_type != pixmap2->pixel_type)
		return 0;

	if (GP_PixmapW(pixmap1) != GP_PixmapW(pixmap2))
		return 0;

	if (GP_PixmapH(pixmap1) != GP_PixmapH(pixmap2))
		return 0;

	GP_Coord x, y, w = GP_PixmapW(pixmap1), h = GP_PixmapH(pixmap1);

	for (x = 0; x < w; x++)
		for (y = 0; y < h; y++)
			if (GP_GetPixel(pixmap1, x, y) != GP_GetPixel(pixmap2, x, y))
				return 0;

	return 1;
}

