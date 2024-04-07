// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_line.h
 * @brief Line drawing algorithms.
 */

#ifndef GFX_GP_LINE_H
#define GFX_GP_LINE_H

#include <core/gp_types.h>

/**
 * @brief Classical Bresenham line drawing algorithm.
 * @ingroup gfx
 *
 * Classical Bresenham with a nice little trick on the top of the original
 * algorithm, we only track half of the line and paint it from both sides at
 * the same time.
 *
 * Draws a line from (x0, y0) to (x1, y1), inclusive. The starting and ending
 * point can be specified in any order (the implementation guarantees that
 * exactly the same set of pixels will be drawn in both cases).
 *
 * @param pixmap A pixmap to draw into.
 * @param x0 A starting point x coordinate.
 * @param y0 A starting point y coordinate.
 * @param x1 An ending point x coordinate.
 * @param y1 An ending point y coordinate.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_line(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
             gp_coord x1, gp_coord y1, gp_pixel pixel);
/**
 * @brief Murphy thick line drawing algorithm.
 * @ingroup gfx
 *
 * This is a modified Bresenham, we run two Bresenham line algorithms, outher
 * one that track the line and inner one that draws lines perpendicular to the
 * line we track. The outher Bresenham also tracks initial errors for the inner
 * one.
 *
 * We do a nice little trick on the top of the original algorithm, we only
 * track half of the line and paint it from both sides at the same time.
 *
 * Draws a line from (x0, y0) to (x1, y1), inclusive. The starting and ending
 * point can be specified in any order (the implementation guarantees that
 * exactly the same set of pixels will be drawn in both cases).
 *
 * @param pixmap A pixmap to draw into.
 * @param x0 A starting point x coordinate.
 * @param y0 A starting point y coordinate.
 * @param x1 An ending point x coordinate.
 * @param y1 An ending point y coordinate.
 * @param r A radius, r=0 is single pixel line.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_line_th(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                gp_coord x1, gp_coord y1, gp_size r, gp_pixel pixel);

void gp_line_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                 gp_coord x1, gp_coord y1, gp_pixel pixel);

void gp_line_th_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                    gp_coord x1, gp_coord y1, gp_size r, gp_pixel pixel);

#endif /* GFX_GP_LINE_H */
