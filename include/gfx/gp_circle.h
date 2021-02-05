// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_CIRCLE_H
#define GP_CIRCLE_H

#include <core/gp_types.h>

/* Circle */

void gp_circle(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
               gp_size r, gp_pixel pixel);

void gp_circle_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                   gp_size r, gp_pixel pixel);

/* Filled Circle */

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
