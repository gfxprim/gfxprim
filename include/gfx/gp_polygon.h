// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GFX_GP_POLYGON_H
#define GFX_GP_POLYGON_H

#include <core/gp_types.h>

void gp_polygon(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
		unsigned int vertex_count, const gp_coord *xy, gp_pixel pixel);

void gp_polygon_raw(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
                    unsigned int vertex_count, const gp_coord *xy, gp_pixel pixel);

void gp_polygon_th_raw(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
                       unsigned int vertex_count, const gp_coord *xy,
                       gp_size r, gp_pixel pixel);

void gp_polygon_th(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
                   unsigned int vertex_count, const gp_coord *xy,
                   gp_size r, gp_pixel pixel);

void gp_fill_polygon(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
                     unsigned int vertex_count, const gp_coord *xy, gp_pixel pixel);

void gp_fill_polygon_raw(gp_pixmap *pixmap, gp_coord x_off, gp_coord y_off,
                         unsigned int vertex_count, const gp_coord *xy, gp_pixel pixel);

#endif /* GFX_GP_POLYGON_H */
