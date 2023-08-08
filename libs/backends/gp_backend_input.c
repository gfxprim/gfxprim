// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_debug.h>
#include <backends/gp_backend_input.h>

void gp_backend_input_destroy(gp_backend *self)
{
	gp_dlist_head *entry, *next_entry;

	GP_DEBUG(1, "Destroying all input drivers");

	for (entry = self->input_drivers.head; entry; entry = next_entry) {
		gp_backend_input *input = GP_LIST_ENTRY(entry, gp_backend_input, list_head);

		next_entry = entry->next;

		input->destroy(input);
	}
}
