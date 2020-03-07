// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GFX_GP_POLYGON_H
#define GFX_GP_POLYGON_H

#include "core/gp_types.h"

void gp_polygon(gp_pixmap *pixmap, unsigned int vertex_count,
                const gp_coord *xy, gp_pixel pixel);

void gp_polygon_raw(gp_pixmap *pixmap, unsigned int vertex_count,
                    const gp_coord *xy, gp_pixel pixel);

void gp_fill_polygon(gp_pixmap *pixmap, unsigned int vertex_count,
                     const gp_coord *xy, gp_pixel pixel);

void gp_fill_polygon_raw(gp_pixmap *pixmap, unsigned int vertex_count,
                         const gp_coord *xy, gp_pixel pixel);

#endif /* GFX_GP_POLYGON_H */
