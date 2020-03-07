// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_ELLIPSE_H
#define GP_ELLIPSE_H

#include "core/gp_types.h"

/* Ellipse */

void gp_ellipse(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                gp_size a, gp_size b, gp_pixel pixel);

void gp_ellipse_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                    gp_size a, gp_size b, gp_pixel pixel);

/* Filled Ellipse */

void gp_fill_ellipse(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                     gp_size a, gp_size b, gp_pixel pixel);

void gp_fill_ellipse_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                         gp_size a, gp_size b, gp_pixel pixel);

#endif /* GP_ELLIPSE_H */
