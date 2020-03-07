// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_ARC_H
#define GP_ARC_H

#include <core/gp_types.h>

#include <math.h>

void gp_arc_segment(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
		    gp_size a, gp_size b, int direction,
		    double start, double end,
		    gp_pixel pixel);

void gp_arc_segment_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
		        gp_size a, gp_size b, int direction,
		        double start, double end,
		        gp_pixel pixel);

#endif /* GP_ARC_H */
