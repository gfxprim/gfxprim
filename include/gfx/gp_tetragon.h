// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_TETRAGON_H
#define GP_TETRAGON_H

#include "core/gp_types.h"

/* Tetragon */

void gp_tetragon(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                 gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
                 gp_coord x3, gp_coord y3, gp_pixel pixel);

void gp_tetragon_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                     gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
                     gp_coord x3, gp_coord y3, gp_pixel pixel);

/* Filled Tetragon */

void gp_fill_tetragon(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                      gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
		      gp_coord x3, gp_coord y3, gp_pixel pixel);

void gp_fill_tetragon_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                          gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2,
                          gp_coord x3, gp_coord y3, gp_pixel pixel);

#endif /* GP_TETRAGON_H */
