// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */
/**
 * @file gp_circle.h.
 * @brief A ring and circle drawing algorithms.
 */
#ifndef GP_CIRCLE_H
#define GP_CIRCLE_H

#include <core/gp_types.h>

/**
 * @brief Draws a circle.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to draw into.
 * @param xcenter A circle center coordinate.
 * @param ycenter A circle center coordinate.
 * @param r A circle radius, r=0 draws a single pixel.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_circle(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
               gp_size r, gp_pixel pixel);

void gp_circle_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                   gp_size r, gp_pixel pixel);

/* Filled Circle */

/**
 * @brief Draws a filled circle.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to draw into.
 * @param xcenter A circle center coordinate.
 * @param ycenter A circle center coordinate.
 * @param r A circle radius, r=0 draws a single pixel.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_fill_circle(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                    gp_size r, gp_pixel pixel);

void gp_fill_circle_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                        gp_size r, gp_pixel pixel);

/* Ring */

void gp_ring(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
             gp_size r1, gp_size r2, gp_pixel pixel);

void gp_ring_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                 gp_size r1, gp_size r2, gp_pixel pixel);

/* Filled Ring */

void gp_fill_ring(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                  gp_size r1, gp_size r2, gp_pixel pixel);

void gp_fill_ring_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                      gp_size r1, gp_size r2, gp_pixel pixel);

#endif /* GP_CIRCLE_H */
