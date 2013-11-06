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

#include "core/GP_Debug.h"

#include "filters/GP_ApplyTables.h"

static GP_Pixel *create_table(const GP_PixelTypeChannel *chan)
{
	size_t table_size = (1 << chan->size);
	GP_Pixel *table = malloc(table_size * sizeof(GP_Pixel));
	GP_Pixel i;

	if (!table) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	for (i = 0; i < table_size; i++)
		table[i] = i;

	return table;
}

static void free_tables(GP_FilterTables *self)
{
	unsigned int i;

	for (i = 0; i < GP_PIXELTYPE_MAX_CHANNELS; i++)
		free(self->table[i]);
}

int GP_FilterTablesInit(GP_FilterTables *self, const GP_Context *ctx)
{
	unsigned int i;
	const GP_PixelTypeDescription *desc;

	GP_DEBUG(2, "Allocating tables for pixel %s",
	         GP_PixelTypeName(ctx->pixel_type));

	for (i = 0; i < GP_PIXELTYPE_MAX_CHANNELS; i++)
		self->table[i] = NULL;

	desc = GP_PixelTypeDesc(ctx->pixel_type);

	for (i = 0; i < desc->numchannels; i++) {
		self->table[i] = create_table(&desc->channels[i]);
		if (!self->table[i]) {
			free_tables(self);
			return 1;
		}
	}

	return 0;
}

GP_FilterTables *GP_FilterTablesAlloc(const GP_Context *ctx)
{
	GP_FilterTables *tables = malloc(sizeof(GP_FilterTables));

	GP_DEBUG(1, "Allocating point filter (%p)", tables);

	if (!tables) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	tables->free_table = 1;

	if (GP_FilterTablesInit(tables, ctx)) {
		free(tables);
		return NULL;
	}

	return tables;
}

void GP_FilterTablesFree(GP_FilterTables *self)
{
	GP_DEBUG(1, "Freeing point filter and tables (%p)", self);

	free_tables(self);

	if (self->free_table)
		free(self);
}
