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

/*

  Common filters typedefs and includes.

 */

#ifndef FILTERS_GP_FILTER_H
#define FILTERS_GP_FILTER_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"

#include "GP_FilterParam.h"

/*
 * Describes filter source, destination and coordinates.
 *
 * For Alloc filter variants dst, dst_x and dst_y is ignored.
 */
typedef struct GP_FilterArea {
	const GP_Context *src;
	GP_Coord src_x, src_y;
	GP_Size src_w, src_h;

	GP_Context *dst;
	GP_Coord dst_x, dst_y;

	void *priv;
} GP_FilterArea;

#define GP_FILTER_AREA_DEFAULT(srcc, dstc)          \
	GP_FilterArea area = {                      \
		.src = srcc,                        \
		.src_x = 0, .src_y = 0,             \
                .src_w = srcc->w, .src_h = srcc->h, \
		.dst = dstc,                        \
		.dst_x = 0, .dst_y = 0,             \
	};

#endif /* FILTERS_GP_FILTER_H */
