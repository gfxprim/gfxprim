// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 */

/**
 * @file gp_transform.h
 * @brief Coordinate transformations for #gp_pixmap swap flags.
 *
 * These transformations are applied before a complex shapes are rendered into
 * a gp_pixmap, e.g. each polygon coordinate is transformed before the polygon
 * rendering algorithm starts.
 */

#ifndef CORE_GP_TRANSFORM_H
#define CORE_GP_TRANSFORM_H

#include <core/gp_common.h>

/**
 * @brief Flips x coordinate within pixmap according to the x_swap flag.
 *
 * @attention Modifies the y variable passed as argument.
 *
 * @param pixmap A pixmap.
 * @param x An x coordinate that may be flipped.
 */
#define GP_TRANSFORM_X(pixmap, x) do {   \
	if ((pixmap)->x_swap)            \
		x = (pixmap)->w - x - 1; \
} while (0)

/**
 * @brief Flips y coordinate within pixmap according to the y_swap flag.
 *
 * @attention Modifies the y variable passed as argument.
 *
 * @param pixmap A pixmap.
 * @param y An y coordinate that may be flipped.
 */
#define GP_TRANSFORM_Y(pixmap, y) do {   \
	if ((pixmap)->y_swap)            \
		y = (pixmap)->h - y - 1; \
} while (0)

/**
 * @brief Swaps coordinates within pixmap according to the axes_swap flag.
 *
 * @attention Modifies the x and y variables passed as argument.
 *
 * @param pixmap A pixmap.
 * @param x An x coordinate that may be swapped with y coordinate.
 * @param y An y coordinate that may be swapped with x coordinate.
 */
#define GP_TRANSFORM_SWAP(pixmap, x, y) do { \
	if ((pixmap)->axes_swap)             \
		GP_SWAP(x, y);               \
} while (0)

/**
 * @brief Does the complete coordinate transformation accordingly to
 *        transformation flags.
 *
 * Uses the gp_pixmap::axes_swap, gp_pixmap::x_swap and gp_pixmap::y_swap flags
 * to transform the coordinates.
 *
 * @attention Modifies the x and y variables passed as argument.
 *
 * @param pixmap A pixmap.
 * @param x An x coordinate that may be transformed.
 * @param y An y coordinate that may be transformed.
 */
#define GP_TRANSFORM_POINT(pixmap, x, y) do { \
	GP_TRANSFORM_SWAP(pixmap, x, y);      \
	GP_TRANSFORM_X(pixmap, x);            \
	GP_TRANSFORM_Y(pixmap, y);            \
} while (0)

/**
 * @brief Transforms a rectangle accordingly to the transformation flags.
 *
 * @attention Modifies the x, y, w, and h variables passed as argument.
 *
 * @param pixmap A pixmap.
 * @param x A left x coordinate of a rectangle.
 * @param y A top y coordinate of a rectangle.
 * @param w A rectangle width.
 * @param h A rectangle height.
 */
#define GP_TRANSFORM_RECT(pixmap, x, y, w, h) do { \
	GP_TRANSFORM_SWAP(pixmap, x, y);           \
	GP_TRANSFORM_SWAP(pixmap, w, h);           \
	                                           \
	if ((pixmap)->x_swap)                      \
		x = (pixmap)->w - x - w;           \
	                                           \
	if ((pixmap)->y_swap)                      \
		y = (pixmap)->h - y - h;           \
} while (0)

/*
 * Transform "user"-coordinates to "real"-coordinates for a blit
 * called as gp_blit(c1, x1, y1, w, h, c2, x2, y2).
 * All x1, y1, x2, y2, w, h are adjusted.
 */
#define GP_TRANSFORM_BLIT(c1, x1, y1, w, h, c2, x2, y2) do { \
	GP_TRANSFORM_RECT(c1, x1, y1, w, h);            \
	int w2 = w, h2 = h;                             \
	GP_TRANSFORM_SWAP(c2, w2, h2);                  \
	GP_TRANSFORM_RECT(c2, x2, y2, w2, h2);          \
} while (0)

/**
 * @brief An inverse transformation to GP_TRANSFORM_POINT().
 *
 * Used to translate mouse pointer coordinates to coordinates on pixmap.
 *
 * @attention Modifies the x and y variables passed as argument.
 *
 * @param pixmap A pixmap.
 * @param x An x coordinate that may be transformed.
 * @param y An y coordinate that may be transformed.
 */
#define GP_RETRANSFORM_POINT(pixmap, x, y) do { \
	GP_TRANSFORM_X(pixmap, x);              \
	GP_TRANSFORM_Y(pixmap, y);              \
	GP_TRANSFORM_SWAP(pixmap, x, y);        \
} while (0)

#endif /* CORE_GP_TRANSFORM_H */
