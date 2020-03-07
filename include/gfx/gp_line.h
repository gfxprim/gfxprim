// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_LINE_H
#define GP_LINE_H

#include "core/gp_types.h"

void gp_line(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
             gp_coord x1, gp_coord y1, gp_pixel pixel);

void gp_line_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                 gp_coord x1, gp_coord y1, gp_pixel pixel);

#endif /* GP_LINE_H */
