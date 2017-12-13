/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef GP_ARC_H
#define GP_ARC_H

#include "core/GP_Pixmap.h"

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
