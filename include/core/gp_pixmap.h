// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_pixmap.h
 * @brief A pixel buffer.
 * @defgroup pixmap Pixmap
 */

#ifndef CORE_GP_PIXMAP_H
#define CORE_GP_PIXMAP_H

#include <stdint.h>
#include <unistd.h>

#include <core/gp_common.h>
#include <core/gp_types.h>
#include <core/gp_pixel.h>
#include <core/gp_gamma_correction.h>

/**
 * @brief A pixmap buffer.
 * @ingroup pixmap
 *
 * This structure holds all information needed for drawing into an pixel
 * buffer.
 */
struct gp_pixmap {
	/** @brief A pointer to image pixels */
	uint8_t *pixels;
	/**
	 * @brief Number of bytes per row
	 *
	 * This is an integer number even when the actual number of bits used
	 * by a row is not a multiple of 8.
	 */
	uint32_t bytes_per_row;
	/** @brief Pixmap width in pixels */
	uint32_t w;
	/** @brief Pixmap height in pixels */
	uint32_t h;
	/**
	 * @brief An offset to apply before the start of the pixel row.
	 *
	 * Offset, in pixels, for non-byte aligned pixels. The x = 0 coordinate
	 * in the pixmap starts at this offset. It's mainly useful for sub pixmaps
	 * for 1BPP, 2BPP and 4BPP pixel types.
	 *
	 * The offset is ignored for byte aligned pixels.
	 */
	uint8_t offset;

	/**
	 * @brief A pixel format.
	 *
	 * See gp_pixel.gen.h and gp_pixel.gen.c.
	 */
	enum gp_pixel_type pixel_type;

	/**
	 * @brief A pointer to a gamma correction table.
	 *
	 * If NULL the channel values are considered linear.
	 */
	gp_gamma *gamma;

	/**
	 * @brief Swaps image x and y axes for drawing.
	 *
	 * In order to draw an image rotated by 90 degrees all that needs to be
	 * done is to set the axes_swap flag before we start drawing into the
	 * pixmap.
	 */
	uint8_t axes_swap:1;
	/**
	 * @brief Mirrors image x axis for drawing.
	 *
	 * In order to draw an image mirrored around x axis all that needs to
	 * be done is to set the axes_swap flag before we start drawing into
	 * the pixmap.
	 */
	uint8_t x_swap:1;
	/**
	 * @brief Mirrors image y axis for drawing.
	 *
	 * In order to draw an image mirrored around y axis all that needs to
	 * be done is to set the axes_swap flag before we start drawing into
	 * the pixmap.
	 */
	uint8_t y_swap:1;

	/** @brief If set pixels are freed on gp_pixmap_free */
	uint8_t free_pixels:1;
};

/**
 * @brief Returns pixel addres in an image.
 *
 * Determines the address of a pixel within the pixmap's buffer.
 *
 * Rows and columns are specified in the image origiinal orientation i.e. this
 * macro ignores the rotation flags.
 */
#define GP_PIXEL_ADDR(pixmap, x, y) ((pixmap)->pixels \
	+ (y) * (pixmap)->bytes_per_row \
	+ ((x + (pixmap)->offset) * gp_pixel_size((pixmap)->pixel_type)) / 8)

#define GP_CALC_ROW_SIZE(pixel_type, width) \
	 ((gp_pixel_size(pixel_type) * width) / 8 + \
	!!((gp_pixel_size(pixel_type) * width) % 8))

/* Performs a series of sanity checks on pixmap, aborting if any fails. */
#define GP_CHECK_PIXMAP(pixmap) do { \
	GP_CHECK(pixmap, "NULL passed as pixmap"); \
	GP_CHECK(pixmap->pixels || pixmap->w == 0 || pixmap->h == 0, "invalid pixmap: pixels NULL on nonzero w h"); \
} while (0)

/**
 * @brief Returns true when pixel is clipped out of pixmap.
 * @ingroup pixmap
 */
#define GP_PIXEL_IS_CLIPPED(pixmap, x, y) \
	((x) < 0 || x >= (typeof(x)) pixmap->w \
	|| (y) < 0 || y >= (typeof(y)) pixmap->h) \

/**
 * @brief Allocates a pixmap.
 * @ingroup pixmap
 *
 * The pixmap consists of two parts, the gp_pixmap structure and pixels array.
 *
 * The rotation flags are set to (0, 0, 0).
 *
 * @param w A pixmap width.
 * @param h A pixmap height.
 * @param type A pixel type.
 *
 * @return A newly allocated pixmap or NULL in a case of malloc() failure.
 */
gp_pixmap *gp_pixmap_alloc(gp_size w, gp_size h, gp_pixel_type type);

/**
 * @brief Sets a correction for the pixmap.
 * @ingroup pixmap
 *
 * If there was a gamma correction allready set for the pixmap the original
 * correction reference counters are decremented and the correction is replaced
 * with a new table.
 *
 * @param self A pixmap.
 * @param corr_type A correction type.
 * @param gamma If applicable the correction gamma.
 *
 * @return Zero on success. May fail if underlying malloc() has failed.
 */
int gp_pixmap_gamma_set(gp_pixmap *self, gp_correction_type corr_type,
                        float gamma);

/**
 * @brief Frees a pixmap.
 * @ingroup pixmap
 *
 * If pixmap->free_pixels is set also free pixel data, this flag is set
 * automatically by gp_pixmap_alloc().
 *
 * @param self A pixmap to free.
 */
void gp_pixmap_free(gp_pixmap *self);

/**
 * @brief A pixmap init flags.
 */
enum gp_pixmap_init_flags {
	/** If set the pixmap->pixels is freed on gp_pixmap_free() */
	GP_PIXMAP_FREE_PIXELS = 0x01,
};

/**
 * @brief Initializes allocated pixmap structure.
 *
 * Initalize pixmap structure, pixels pointer is not dereferenced so it's safe
 * to pass NULL there and allocate it later with size pixmap->bpr * pixmap->h.
 *
 * BEWARE: The user has to make sure that the pixels array has correct size and
 *         format.
 *
 * @param pixmap A pointer to a pixmap structure to be initialized with a data.
 * @param w Pixmap width
 * @param h Pixmap height
 * @param type A pixel type, describes how pixels are organized in the data buffer.
 * @param pixels A pointer to a buffer with the pixel data.
 * @param flags A bitmask or of the enum gp_pixmap_init_flags
 *
 * @return Returns pointer to pixmap that is passed as first argument.
 */
gp_pixmap *gp_pixmap_init(gp_pixmap *pixmap, gp_size w, gp_size h,
                          gp_pixel_type type, void *pixels,
                          enum gp_pixmap_init_flags flags);

/**
 * @brief Creates a pixmap from a buffer allocated by malloc().
 * @ingroup pixmap
 *
 * This is actually shorthand for allocating the gp_pixmap structure and
 * calling gp_pixmap_init() on the resulting pointer.
 *
 * @attention The user has to make sure that the pixels array has correct size
 *            and format.
 *
 * @param w Pixmap width
 * @param h Pixmap height
 * @param type A pixel type, describes how pixels are organized in the data buffer.
 * @param pixels A pointer to a buffer with the pixel data.
 * @param flags A bitmask or of the enum gp_pixmap_init_flags
 *
 * @return A newly allocated and initialized pixmap or NULL in a case of a failure.
 */
static inline gp_pixmap *gp_pixmap_from_data(gp_size w, gp_size h,
                                             gp_pixel_type type, void *pixels,
					     enum gp_pixmap_init_flags flags)
{
	gp_pixmap *ret = malloc(sizeof(gp_pixmap));

	if (!ret)
		return NULL;

	return gp_pixmap_init(ret, w, h, type, pixels, flags);
}

/**
 * @brief Resizes pixmap.
 *
 * This only resizes the buffer, the pixel values are uninitialized after the
 * operation!
 *
 * If you are looking for functions to resize an image you are looking for
 * resampling filters.
 *
 * @param self A pixmap.
 * @param w A new width.
 * @param h A new height.
 *
 * @returns Zero on success non-zero on failure if underlying malloc() has
 *          failed.
 */
int gp_pixmap_resize(gp_pixmap *self, gp_size w, gp_size h);

/**
 * @brief A pixmap copy flags.
 */
enum gp_pixmap_copy_flags {
	/** Copy bitmap pixels too. If not set pixels are uninitalized */
	GP_PIXMAP_COPY_PIXELS   = 0x01,
	/** Copy image rotation flags. If not set flags are set to (0, 0, 0) */
	GP_PIXMAP_COPY_ROTATION = 0x02,
	/** Copy image gamma. If not the resulting image has linear channes. */
	GP_PIXMAP_COPY_GAMMA = 0x04,
};

/**
 * @brief Copies a pixmap.
 *
 * Allocates a pixmap with exactly same data as source pixmap.
 *
 * @param src An input pixmap.
 * @param flags An enum gp_pixmap_copy_flags.
 * @return A newly allocated and initialized pixmap or NULL in a case of a failure.
 */
gp_pixmap *gp_pixmap_copy(const gp_pixmap *src, enum gp_pixmap_copy_flags flags);

/**
 * @brief Initializes a subpixmap.
 *
 * The subpixmap has to fit into the source pixmap!
 *
 * The original image must not be freed including its gamma tables until the
 * resulting subpixmap is being in use.
 *
 * @param src An input pixmap.
 * @param subpixmap A pointer to a pixmap to initialize.
 * @param x A starting coordinate of the subpixmap.
 * @param y A starting coordinate of the subpixmap.
 * @param w A subpixmap width.
 * @param h A subpixmap height.
 *
 * @return The pointer passed as subpixmap argument.
 */
gp_pixmap *gp_sub_pixmap(const gp_pixmap *src, gp_pixmap *subpixmap,
                         gp_coord x, gp_coord y, gp_size w, gp_size h);

/**
 * @brief Allocate and initalize a subpixmap.
 *
 * The subpixmap has to fit into the source pixmap!
 *
 * The original image must not be freed including its gamma tables until the
 * resulting subpixmap is being in use.
 *
 * @param src An input pixmap.
 * @param x A starting coordinate of the subpixmap.
 * @param y A starting coordinate of the subpixmap.
 * @param w A subpixmap width.
 * @param h A subpixmap height.
 *
 * @return A newly allocated and initialized subpixmap or NULL in a case of
 *         failure.
 */
gp_pixmap *gp_sub_pixmap_alloc(const gp_pixmap *src,
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

/**
 * @brief Prints pixmap information into stdout.
 *
 * @param self A pixmap to print an info about.
 */
void gp_pixmap_print_info(const gp_pixmap *self);

/**
 * @brief Rotates pixmap flags clockwise.
 *
 * @param self A pixmap.
 *
 * The underlying pixmap buffer is not changed, only the rotation flags are
 * affected.
 */
void gp_pixmap_rotate_cw(gp_pixmap *self);

/**
 * @brief Rotates pixmap flags counter clock wise.
 *
 * @param self A pixmap.
 *
 * The underlying pixmap buffer is not changed, only the rotation flags are
 * affected.
 */
void gp_pixmap_rotate_ccw(gp_pixmap *self);

/**
 * @brief Compares rotation flags for two pixmaps.
 *
 * @param c1 A pixmap.
 * @param c2 A pixmap.
 *
 * @return True if rotation flags are equal.
 */
static inline int gp_pixmap_rotation_equal(const gp_pixmap *c1,
                                           const gp_pixmap *c2)
{
	return c1->axes_swap == c2->axes_swap &&
	       c1->x_swap == c2->x_swap &&
	       c1->y_swap == c2->y_swap;
}

/**
 * @brief Sets pixmap rotation flags.
 *
 * @param self A pixmap.
 * @param axes_swap Set to 1 if axes should be swapped.
 * @param x_swap Set to 1 if x axis should be mirrored.
 * @param y_swap Set to 1 if y axis should be mirrored.
 */
static inline void gp_pixmap_rotation_set(gp_pixmap *self, int axes_swap,
                                          int x_swap, int y_swap)
{
	self->axes_swap = axes_swap;
	self->x_swap = x_swap;
	self->y_swap = y_swap;
}

/**
 * @brief Copies rotation flags from one pixmap to another.
 *
 * @param src A source pixmap for rotation flags.
 * @param dst A destination pixmap for rotation flags.
 */
static inline void gp_pixmap_rotation_copy(const gp_pixmap *src,
                                           gp_pixmap *dst)
{
	dst->axes_swap = src->axes_swap;
	dst->x_swap = src->x_swap;
	dst->y_swap = src->y_swap;
}

/**
 * @brief Returns pixmap width taking axes swap into account.
 *
 * @param self A pixmap.
 * @return A pixmap width taking axes swap into an account.
 */
static inline gp_size gp_pixmap_w(const gp_pixmap *self)
{
	if (self->axes_swap)
		return self->h;
	else
		return self->w;
}

/**
 * @brief Returns pixmap height taking axes swap into account.
 *
 * @param self A pixmap.
 * @return A pixmap height taking axes swap into an account.
 */
static inline gp_size gp_pixmap_h(const gp_pixmap *self)
{
	if (self->axes_swap)
		return self->w;
	else
		return self->h;
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

