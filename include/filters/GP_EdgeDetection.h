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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef FILTERS_GP_EDGE_DETECTION_H
#define FILTERS_GP_EDGE_DETECTION_H

#include <filters/GP_Filter.h>

int gp_filter_edge_sobel(const gp_pixmap *src,
                         gp_pixmap **E, gp_pixmap **Phi,
                         gp_progress_cb *callback);

int gp_filter_edge_prewitt(const gp_pixmap *src,
                           gp_pixmap **E, gp_pixmap **Phi,
                           gp_progress_cb *callback);


#endif /* FILTERS_GP_EDGE_DETECTION_H */
