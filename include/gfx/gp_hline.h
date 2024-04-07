// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_hline.h
 * @brief A horizontal line drawing algorithm.
 */
#ifndef GFX_GP_HLINE_H
#define GFX_GP_HLINE_H

#include <core/gp_types.h>
#include <gfx/gp_hline.gen.h>

/**
 * @brief Horizontal line drawing algorithm.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to draw into.
 * @param x0 A starting point x coordinate.
 * @param x1 An ending point x coordinate.
 * @param y A y coordinate.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_hline_xxy(gp_pixmap *pixmap, gp_coord x0, gp_coord x1, gp_coord y,
                  gp_pixel pixel);

void gp_hline_xxy_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord x1,
                      gp_coord y, gp_pixel pixel);

/**
 * @brief Horizontal line drawing algorithm.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to draw into.
 * @param x A starting point x coordinate.
 * @param w A line width, a lenght of the line.
 * @param y A y coordinate.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_hline_xyw(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size w,
                  gp_pixel pixel);

void gp_hline_xyw_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size w,
                      gp_pixel pixel);

/* default argument set is xxy */
static inline void gp_hline_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord x1,
                                gp_coord y, gp_pixel p)
{
	gp_hline_xxy_raw(pixmap, x0, x1, y, p);
}

/**
 * @brief Horizontal line drawing algorithm.
 * @ingroup gfx
 *
 * An alias for gp_hline_xxy().
 */
static inline void gp_hline(gp_pixmap *pixmap, gp_coord x0, gp_coord x1,
                            gp_coord y, gp_pixel p)
{
	gp_hline_xxy(pixmap, x0, x1, y, p);
}

#endif /* GFX_GP_HLINE_H */
