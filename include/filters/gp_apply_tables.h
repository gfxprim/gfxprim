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

  Applies per-channel tables on a pixmap pixels. Used for fast point filters
  implementation.

 */

#ifndef FILTERS_GP_APPLY_TABLES_H
#define FILTERS_GP_APPLY_TABLES_H

#include <filters/gp_filter.h>

/*
 * Per-channel lookup tables.
 */
typedef struct gp_filter_tables {
	gp_pixel *table[GP_PIXELTYPE_MAX_CHANNELS];
	int free_table:1;
} gp_filter_tables;

/*
 * Generic point filter, applies corresponding table on bitmap.
 */
int gp_filter_tables_apply(const gp_pixmap *const src,
                           gp_coord x_src, gp_coord y_src,
                           gp_size w_src, gp_size h_src,
			   gp_pixmap *dst,
			   gp_coord x_dst, gp_coord y_dst,
			   const gp_filter_tables *const tables,
			   gp_progress_cb *callback);

/*
 * Aloocates and initializes tables.
 */
int gp_filter_tables_init(gp_filter_tables *self, const gp_pixmap *pixmap);

/*
 * Allocates and initializes table structure and tables.
 */
gp_filter_tables *gp_filter_tables_alloc(const gp_pixmap *pixmap);

/*
 * Frees point filter tables.
 */
void gp_filter_tables_free(gp_filter_tables *self);

#endif /* FILTERS_GP_APPLY_TABLES_H */
