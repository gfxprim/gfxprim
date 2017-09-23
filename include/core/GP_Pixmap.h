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

#ifndef CORE_GP_PIXMAP_H
#define CORE_GP_PIXMAP_H

#include <stdint.h>
#include <unistd.h>

#include "GP_Common.h"
#include "GP_Types.h"
#include "GP_Pixel.h"

struct GP_Gamma;

/* This structure holds all information needed for drawing into an image. */
typedef struct GP_Pixmap {
	uint8_t *pixels;	 /* pointer to image pixels */
	uint8_t bpp;		 /* pixel size in bits */
	uint32_t bytes_per_row;
	uint32_t w;		 /* width in pixels */
	uint32_t h;		 /* height in pixels */
	/*
	 * Row bit offset. The offset is ignored for byte aligned pixels.
	 * Basically it's used for non aligned pixels with combination
	 * with subpixmapes.
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
	uint8_t free_pixels:1;  /* If set pixels are freed on GP_PixmapFree */
} GP_Pixmap;

/* Determines the address of a pixel within the pixmap's image.
 * Rows and columns are specified in the image's orientation
 * (i.e. they might not be XY if the image is rotated).
 */
#define GP_PIXEL_ADDR(pixmap, x, y) ((pixmap)->pixels \
	+ (y) * (pixmap)->bytes_per_row \
	+ ((x) * (pixmap)->bpp) / 8)

#define GP_CALC_ROW_SIZE(pixel_type, width) \
	 ((GP_PixelSize(pixel_type) * width) / 8 + \
	!!((GP_PixelSize(pixel_type) * width) % 8))

/* Performs a series of sanity checks on pixmap, aborting if any fails. */
#define GP_CHECK_PIXMAP(pixmap) do { \
	GP_CHECK(pixmap, "NULL passed as pixmap"); \
	GP_CHECK(pixmap->pixels, "invalid pixmap: NULL image pointer"); \
	GP_CHECK(pixmap->bpp <= 32, "invalid pixmap: unsupported bits-per-pixel count"); \
	GP_CHECK(pixmap->w > 0 && pixmap->h > 0, "invalid pixmap: invalid image size"); \
} while (0)

/*
 * Is true, when pixel is clipped out of pixmap.
 */
#define GP_PIXEL_IS_CLIPPED(pixmap, x, y) \
	((x) < 0 || x >= (typeof(x)) pixmap->w \
	|| (y) < 0 || y >= (typeof(y)) pixmap->h) \

/*
 * Allocate pixmap.
 *
 * The pixmap consists of two parts, the GP_Pixmap structure and pixels array.
 *
 * The rotation flags are set to (0, 0, 0).
 */
GP_Pixmap *GP_PixmapAlloc(GP_Size w, GP_Size h, GP_PixelType type);

/*
 * Free pixmap.
 *
 * If pixmap->free_pixels, also free pixel data.
 */
void GP_PixmapFree(GP_Pixmap *pixmap);

/*
 * Initalize pixmap, pixels pointer is not dereferenced so it's safe to pass
 * NULL there and allocate it later with size pixmap->bpr * pixmap->h.
 *
 * The returned pointer is the pointer you passed as first argument.
 */
GP_Pixmap *GP_PixmapInit(GP_Pixmap *pixmap, GP_Size w, GP_Size h,
                           GP_PixelType type, void *pixels);

/*
 * Resizes pixmap->pixels array and changes metadata to match the new size.
 *
 * Returns non-zero on failure (malloc() has failed).
 *
 * This call only resizes the pixel array. The pixel values, after resizing,
 * are __UNINITALIZED__ use resampling filters to resize image data.
 */
int GP_PixmapResize(GP_Pixmap *pixmap, GP_Size w, GP_Size h);

enum GP_PixmapCopyFlags {
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
 * Allocates a contex with exactly same values as source pixmap.
 */
GP_Pixmap *GP_PixmapCopy(const GP_Pixmap *src, int flags);

/*
 * Initalize subpixmap. The returned pointer points to passed subpixmap.
 */
GP_Pixmap *GP_SubPixmap(const GP_Pixmap *pixmap, GP_Pixmap *subpixmap,
                          GP_Coord x, GP_Coord y, GP_Size w, GP_Size h);

/*
 * Allocate and initalize subpixmap.
 *
 * The free_pixels flag is set to 0 upon subpixmap initalization so the
 * GP_PixmapFree() would not call free() upon the subpixmap->pixels pointer.
 */
GP_Pixmap *GP_SubPixmapAlloc(const GP_Pixmap *pixmap,
                               GP_Coord x, GP_Coord y, GP_Size w, GP_Size h);

/*
 * Converts pixmap to a different pixel type.
 * Returns a newly allocated pixmap.
 *
 * This is naive implementation that doesn't do any ditherings or error
 * diffusions.
 */
GP_Pixmap *GP_PixmapConvertAlloc(const GP_Pixmap *src,
                                   GP_PixelType dst_pixel_type);

/*
 * Converts pixmap to a different pixel type.
 *
 * This is naive implementation that doesn't do any ditherings or error
 * diffusions.
 */
GP_Pixmap *GP_PixmapConvert(const GP_Pixmap *src, GP_Pixmap *dst);

/*
 * Prints pixmap information into stdout.
 */
void GP_PixmapPrintInfo(const GP_Pixmap *self);

/*
 * Rotates pixmap flags clock wise.
 */
void GP_PixmapRotateCW(GP_Pixmap *pixmap);

/*
 * Rotates pixmap flags counter clock wise.
 */
void GP_PixmapRotateCCW(GP_Pixmap *pixmap);

/*
 * Retruns 1 if rotation flags are equal.
 */
static inline int GP_PixmapRotationEqual(const GP_Pixmap *c1,
                                          const GP_Pixmap *c2)
{
	return c1->axes_swap == c2->axes_swap &&
	       c1->x_swap == c2->x_swap &&
	       c1->y_swap == c2->y_swap;
}

/*
 * Sets rotation flags.
 */
static inline void GP_PixmapSetRotation(GP_Pixmap *dst, int axes_swap,
                                         int x_swap, int y_swap)
{
	dst->axes_swap = axes_swap;
	dst->x_swap = x_swap;
	dst->y_swap = y_swap;
}

/*
 * Copies rotation flags.
 */
static inline void GP_PixmapCopyRotation(const GP_Pixmap *src,
                                          GP_Pixmap *dst)
{
	dst->axes_swap = src->axes_swap;
	dst->x_swap = src->x_swap;
	dst->y_swap = src->y_swap;
}

/*
 * Returns pixmap width and height taking the rotation flags into a account.
 */
static inline GP_Size GP_PixmapW(const GP_Pixmap *pixmap)
{
	if (pixmap->axes_swap)
		return pixmap->h;
	else
		return pixmap->w;
}

static inline GP_Size GP_PixmapH(const GP_Pixmap *pixmap)
{
	if (pixmap->axes_swap)
		return pixmap->w;
	else
		return pixmap->h;
}

/*
 * Compare two pixmaps. Returns true only if all of types, sizes and
 * bitmap data match. Takes transformations into account.
 *
 * For now ignores gamma tables.
 *
 * Currently rather slow (getpixel).
 * TODO: speed up for same rotation and same bit-offset data (per-row memcpy).
 */

int GP_PixmapEqual(const GP_Pixmap *pixmap1, const GP_Pixmap *pixmap2);

#endif /* CORE_GP_PIXMAP_H */

