// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

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
