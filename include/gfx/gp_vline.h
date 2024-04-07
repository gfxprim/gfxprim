// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_vline.h
 * @brief A vertical line drawing algorithm.
 */
#ifndef GFX_GP_VLINE_H
#define GFX_GP_VLINE_H

#include <core/gp_types.h>
#include <gfx/gp_vline.gen.h>

/**
 * @brief Vertical line drawing algorithm.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to draw into.
 * @param x A x coordinate.
 * @param y0 A starting point y coordinate.
 * @param y1 An ending point y coordinate.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_vline_xyy(gp_pixmap *pixmap, gp_coord x, gp_coord y0,
                  gp_coord y1, gp_pixel pixel);

void gp_vline_xyy_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y0,
                      gp_coord y1, gp_pixel pixel);

/**
 * @brief Vertical line drawing algorithm.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to draw into.
 * @param x A x coordinate.
 * @param y A starting point y coordinate.
 * @param h A line height, a lenght of the line.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_vline_xyh(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size h,
                  gp_pixel pixel);

void gp_vline_xyh_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size h,
                      gp_pixel pixel);

/**
 * @brief Horizontal line drawing algorithm.
 * @ingroup gfx
 *
 * An alias for gp_hline_xyy().
 */
static inline void gp_vline(gp_pixmap *pixmap, gp_coord x,
                            gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	gp_vline_xyy(pixmap, x, y0, y1, pixel);
}

static inline void gp_vline_raw(gp_pixmap *pixmap, gp_coord x,
                                gp_coord y0, gp_coord y1, gp_pixel pixel)
{
	gp_vline_xyy_raw(pixmap, x, y0, y1, pixel);
}

#endif /* GFX_GP_VLINE_H */
