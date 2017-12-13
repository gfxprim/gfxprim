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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Data storage operations example.

  */

#include <stdio.h>

#include <gfxprim.h>

int main(void)
{
	gp_storage *storage = gp_storage_create();
	gp_data_node *flags;

	if (storage == NULL)
		return 1;

	printf("Empty storage -----------------------\n");
	gp_data_print(gp_storage_root(storage));
	printf("-------------------------------------\n\n");

	gp_data_node data = {
		.type = GP_DATA_INT,
		.value.i = 300,
		.id = "dpi"
	};

	gp_storage_add(storage, NULL, &data);

	gp_storage_add_string(storage, NULL, "orientation", "top-down");

	printf("Flat storage ------------------------\n");
	gp_data_print(gp_storage_root(storage));
	printf("-------------------------------------\n\n");

	flags = gp_storage_add_dict(storage, NULL, "flags");

	data.type = GP_DATA_INT;
	data.id = "compression_level";
	data.value.i = 10;

	gp_storage_add(storage, flags, &data);

	printf("Recursive storage -------------------\n");
	gp_data_print(gp_storage_root(storage));
	printf("-------------------------------------\n\n");

	gp_data_print(gp_storage_get(storage, NULL, "dpi"));
	gp_storage_destroy(storage);

	return 0;
}
