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

  Applies per-channel tables on a context pixels. Used for fast point filters
  implementation.

 */

#ifndef FILTERS_GP_APPLY_TABLES_H
#define FILTERS_GP_APPLY_TABLES_H

#include "GP_Filter.h"

/*
 * Per-channel lookup tables.
 */
typedef struct GP_FilterTables {
	GP_Pixel *table[GP_PIXELTYPE_MAX_CHANNELS];
	int free_table:1;
} GP_FilterTables;

/*
 * Generic point filter, applies corresponding table on bitmap.
 */
int GP_FilterTablesApply(const GP_FilterArea *const area,
                         const GP_FilterTables *const tables,
                         GP_ProgressCallback *callback);

/*
 * Aloocates and initializes tables.
 */
int GP_FilterTablesInit(GP_FilterTables *self, const GP_Context *ctx);

/*
 * Allocates and initializes table structure and tables.
 */
GP_FilterTables *GP_FilterTablesAlloc(const GP_Context *ctx);

/*
 * Frees point filter tables.
 */
void GP_FilterTablesFree(GP_FilterTables *self);

#endif /* FILTERS_GP_APPLY_TABLES_H */
