// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_common.h>
#include <input/gp_keys.h>
#include "keys.h"

int gp_ev_key_val(const char *name)
{
	int left  = 0;
	int right = GP_ARRAY_SIZE(keys);
	int oleft, oright;
	int cmp;

	do {
		if (!(cmp = strcasecmp(name, keys[(left+right)/2].name)))
			return keys[(left+right)/2].val;

		oleft  = left;
		oright = right;

		if (cmp > 0)
			left = (left+right)/2;
		else
			right = (left+right)/2;

	} while (left != oleft || right != oright);

	return -1;
}

const char *gp_ev_key_name(unsigned int key)
{
	if (key >= GP_ARRAY_SIZE(key_names))
		return "Unknown";

	if (!key_names[key])
		return "Unknown";

	return key_names[key];
}
