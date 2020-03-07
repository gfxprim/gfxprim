// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef CORE_GP_PIXMAP_H
#define CORE_GP_PIXMAP_H

#include <stdint.h>
#include <unistd.h>

#include "core/gp_common.h"
#include "core/gp_types.h"
#include <core/gp_pixel.h>

/* This structure holds all information needed for drawing into an image. */
struct gp_pixmap {
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
	enum gp_pixel_type pixel_type;

	/*
	 * Pointer to optional Gamma table.
	 *
	 * If NULL, the channel values are considered linear.
	 *
	 * See GP_GammaCorrection.h.
	 */
	struct gp_gamma *gamma;

	/*
	 * Image orientation. Most common is landscape (0, 0, 0),
	 * portrait with normal topleft corner is (1, 0, 0).
	 */
	uint8_t axes_swap:1;	/* swap axes so that x is y and y is x */
	uint8_t x_swap:1;	/* swap direction on x */
	uint8_t y_swap:1;	/* swap direction on y */
	uint8_t bit_endian:1;	/* GP_BIT_ENDIAN */
	uint8_t free_pixels:1;  /* If set pixels are freed on gp_pixmap_free */
};

/* Determines the address of a pixel within the pixmap's image.
 * Rows and columns are specified in the image's orientation
 * (i.e. they might not be XY if the image is rotated).
 */
#define GP_PIXEL_ADDR(pixmap, x, y) ((pixmap)->pixels \
	+ (y) * (pixmap)->bytes_per_row \
	+ ((x) * (pixmap)->bpp) / 8)

#define GP_CALC_ROW_SIZE(pixel_type, width) \
	 ((gp_pixel_size(pixel_type) * width) / 8 + \
	!!((gp_pixel_size(pixel_type) * width) % 8))

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
 * The pixmap consists of two parts, the gp_pixmap structure and pixels array.
 *
 * The rotation flags are set to (0, 0, 0).
 */
gp_pixmap *gp_pixmap_alloc(gp_size w, gp_size h, gp_pixel_type type);

/*
 * Sets gamma for the pixmap.
 */
int gp_pixmap_set_gamma(gp_pixmap *self, float gamma);

/*
 * Free pixmap.
 *
 * If pixmap->free_pixels, also free pixel data.
 */
void gp_pixmap_free(gp_pixmap *pixmap);

/*
 * Initalize pixmap, pixels pointer is not dereferenced so it's safe to pass
 * NULL there and allocate it later with size pixmap->bpr * pixmap->h.
 *
 * The returned pointer is the pointer you passed as first argument.
 */
gp_pixmap *gp_pixmap_init(gp_pixmap *pixmap, gp_size w, gp_size h,
                          gp_pixel_type type, void *pixels);

/*
 * Resizes pixmap->pixels array and changes metadata to match the new size.
 *
 * Returns non-zero on failure (malloc() has failed).
 *
 * This call only resizes the pixel array. The pixel values, after resizing,
 * are __UNINITALIZED__ use resampling filters to resize image data.
 */
int gp_pixmap_resize(gp_pixmap *pixmap, gp_size w, gp_size h);

enum gp_pixmap_copy_flags {
	/* Copy bitmap pixels too. If not set pixels are uninitalized */
	GP_COPY_WITH_PIXELS   = 0x01,
	/* Copy image rotation flags. If not set flags are set to (0, 0, 0) */
	GP_COPY_WITH_ROTATION = 0x02,
};

/*
 * Allocates a contex with exactly same values as source pixmap.
 */
gp_pixmap *gp_pixmap_copy(const gp_pixmap *src, int flags);

/*
 * Initalize subpixmap. The returned pointer points to passed subpixmap.
 */
gp_pixmap *gp_sub_pixmap(const gp_pixmap *pixmap, gp_pixmap *subpixmap,
                         gp_coord x, gp_coord y, gp_size w, gp_size h);

/*
 * Allocate and initalize subpixmap.
 *
 * The free_pixels flag is set to 0 upon subpixmap initalization so the
 * gp_pixmap_free() would not call free() upon the subpixmap->pixels pointer.
 */
gp_pixmap *gp_sub_pixmap_alloc(const gp_pixmap *pixmap,
                               gp_coord x, gp_coord y, gp_size w, gp_size h);

/*
 * Converts pixmap to a different pixel type.
 * Returns a newly allocated pixmap.
 *
 * This is naive implementation that doesn't do any ditherings or error
 * diffusions.
 */
gp_pixmap *gp_pixmap_convert_alloc(const gp_pixmap *src,
                                   gp_pixel_type dst_pixel_type);

/*
 * Converts pixmap to a different pixel type.
 *
 * This is naive implementation that doesn't do any ditherings or error
 * diffusions.
 */
gp_pixmap *gp_pixmap_convert(const gp_pixmap *src, gp_pixmap *dst);

/*
 * Prints pixmap information into stdout.
 */
void gp_pixmap_print_info(const gp_pixmap *self);

/*
 * Rotates pixmap flags clock wise.
 */
void gp_pixmap_rotate_cw(gp_pixmap *pixmap);

/*
 * Rotates pixmap flags counter clock wise.
 */
void gp_pixmap_rotate_ccw(gp_pixmap *pixmap);

/*
 * Retruns 1 if rotation flags are equal.
 */
static inline int gp_pixmap_rotation_equal(const gp_pixmap *c1,
                                           const gp_pixmap *c2)
{
	return c1->axes_swap == c2->axes_swap &&
	       c1->x_swap == c2->x_swap &&
	       c1->y_swap == c2->y_swap;
}

/*
 * Sets rotation flags.
 */
static inline void gp_pixmap_set_rotation(gp_pixmap *dst, int axes_swap,
                                          int x_swap, int y_swap)
{
	dst->axes_swap = axes_swap;
	dst->x_swap = x_swap;
	dst->y_swap = y_swap;
}

/*
 * Copies rotation flags.
 */
static inline void gp_pixmap_copy_rotation(const gp_pixmap *src,
                                           gp_pixmap *dst)
{
	dst->axes_swap = src->axes_swap;
	dst->x_swap = src->x_swap;
	dst->y_swap = src->y_swap;
}

/*
 * Returns pixmap width and height taking the rotation flags into a account.
 */
static inline gp_size gp_pixmap_w(const gp_pixmap *pixmap)
{
	if (pixmap->axes_swap)
		return pixmap->h;
	else
		return pixmap->w;
}

static inline gp_size gp_pixmap_h(const gp_pixmap *pixmap)
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

int gp_pixmap_equal(const gp_pixmap *pixmap1, const gp_pixmap *pixmap2);

#endif /* CORE_GP_PIXMAP_H */

