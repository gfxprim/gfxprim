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

static gp_pixel *create_table(const gp_pixel_channel *chan)
{
	size_t table_size = (1 << chan->size);
	gp_pixel *table = malloc(table_size * sizeof(gp_pixel));
	gp_pixel i;

	GP_DEBUG(2, "Table for channel '%s' size %zu (%p)",
	         chan->name, table_size, table);

	if (!table) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	for (i = 0; i < table_size; i++)
		table[i] = i;

	return table;
}

static void free_tables(gp_filter_tables *self)
{
	unsigned int i;

	for (i = 0; i < GP_PIXELTYPE_MAX_CHANNELS; i++) {

		if (!self->table[i])
			break;

		GP_DEBUG(2, "Freeing table (%p)", self->table[i]);
		free(self->table[i]);
	}
}

int gp_filter_tables_init(gp_filter_tables *self, const gp_pixmap *pixmap)
{
	unsigned int i;
	const gp_pixel_type_desc *desc;

	GP_DEBUG(2, "Allocating tables for pixel %s",
	         gp_pixel_type_name(pixmap->pixel_type));

	for (i = 0; i < GP_PIXELTYPE_MAX_CHANNELS; i++)
		self->table[i] = NULL;

	desc = gp_pixel_desc(pixmap->pixel_type);

	for (i = 0; i < desc->numchannels; i++) {
		self->table[i] = create_table(&desc->channels[i]);
		if (!self->table[i]) {
			free_tables(self);
			return 1;
		}
	}

	self->free_table = 0;

	return 0;
}

gp_filter_tables *gp_filter_tables_alloc(const gp_pixmap *pixmap)
{
	gp_filter_tables *tables = malloc(sizeof(gp_filter_tables));

	GP_DEBUG(1, "Allocating point filter (%p)", tables);

	if (!tables) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	if (gp_filter_tables_init(tables, pixmap)) {
		free(tables);
		return NULL;
	}

	tables->free_table = 1;

	return tables;
}

void gp_filter_tables_free(gp_filter_tables *self)
{
	GP_DEBUG(1, "Freeing point filter and tables (%p)", self);

	free_tables(self);

	if (self->free_table) {
		GP_DEBUG(2, "Freeing table itself");
		free(self);
	}
}
