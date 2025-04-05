// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_types.h>

static const char *symmetry_names[] = {
	"90",
	"180",
	"270",
	"H",
	"V",
	NULL,
};

const char **gp_symmetry_names = symmetry_names;

gp_symmetry gp_symmetry_by_name(const char *symmetry)
{
	int i;

	for (i = 0; symmetry_names[i] != NULL; i++)
		if (!strcasecmp(symmetry, symmetry_names[i]))
			return i;

	return -1;
}
