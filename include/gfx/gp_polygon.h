// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_polygon.h
 * @brief A polygon drawing algorithm.
 */
#ifndef GFX_GP_POLYGON_H
#define GFX_GP_POLYGON_H

#include <core/gp_types.h>

/**
 * @brief Draws a polygon outline.
 * @ingroup gfx
 *
 * Traces a polygon by drawing lines between subsequent coordinates with gp_line().
 *
 * @param pixmap A pixmap to draw the polygon into.
 * @param x_off A x offset to draw the polygon at.
 * @param y_off A y offset to draw the polygon at.
 * @param vertex_count The number of coordinates in the xy array.
 * @param xy An array of a 2 * vertex_count numbers in the [x0, y0, ..., xn, yn] format.
 * @param pixel A pixel value to be used to draw the polygon.
 */
void gp_polygon(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
		unsigned int vertex_count, const gp_coord *xy, gp_pixel pixel);

void gp_polygon_raw(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
                    unsigned int vertex_count, const gp_coord *xy, gp_pixel pixel);

void gp_polygon_th_raw(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
                       unsigned int vertex_count, const gp_coord *xy,
                       gp_size r, gp_pixel pixel);

/**
 * @brief Draws a thick polygon outline.
 * @ingroup gfx
 *
 * Traces a polygon by drawing lines between subsequent coordinates with gp_line_th().
 *
 * @param pixmap A pixmap to draw the polygon into.
 * @param x_off A x offset to draw the polygon at.
 * @param y_off A y offset to draw the polygon at.
 * @param vertex_count The number of coordinates in the xy array.
 * @param xy An array of a 2 * vertex_count numbers in the [x0, y0, ..., xn, yn] format.
 * @param r A radius, r=0 is single pixel line.
 * @param pixel A pixel value to be used to draw the polygon.
 */
void gp_polygon_th(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
                   unsigned int vertex_count, const gp_coord *xy,
                   gp_size r, gp_pixel pixel);

/**
 * @brief Fills a polygon.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to draw the polygon into.
 * @param x_off A x offset to draw the polygon at.
 * @param y_off A y offset to draw the polygon at.
 * @param vertex_count The number of coordinates in the xy array.
 * @param xy An array of a 2 * vertex_count numbers in the [x0, y0, ..., xn, yn] format.
 * @param pixel A pixel value to be used to draw the polygon.
 */
void gp_fill_polygon(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
                     unsigned int vertex_count, const gp_coord *xy, gp_pixel pixel);

void gp_fill_polygon_raw(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
                         unsigned int vertex_count, const gp_coord *xy, gp_pixel pixel);

#endif /* GFX_GP_POLYGON_H */
