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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Very basic GP_Context tests.

 */

#include <core/GP_Context.h>

#include "tst_test.h"

/*
 * Check that Context is correctly allocated and freed
 */
static int Context_Alloc_Free(void)
{
	GP_Context *c;

	c = GP_ContextAlloc(100, 200, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_report(0, "GP_ContextAlloc() failed");
		return TST_FAILED;
	}

	/* Assert context properties */
	if (c->bpp != 24) {
		tst_report(0, "Context->bpp != 24 (== %i)", c->bpp);
		return TST_FAILED;
	}

	if (c->bytes_per_row != 3 * c->w) {
		tst_report(0, "Context->bytes_per_row != %i (== %i)",
		           3 * c->w, c->bytes_per_row);
		return TST_FAILED;
	}

	if (c->w != 100) {
		tst_report(0, "Context->w != 100 (== %i)", c->w);
		return TST_FAILED;
	}
	
	if (c->h != 200) {
		tst_report(0, "Context->h != 200 (== %i)", c->h);
		return TST_FAILED;
	}
	
	if (c->offset != 0) {
		tst_report(0, "Context->offset != 0");
		return TST_FAILED;
	}

	if (c->pixel_type != GP_PIXEL_RGB888) {
		tst_report(0, "Context->pixel_type != GP_PIXEL_RGB888");
		return TST_FAILED;
	}

	if (c->gamma != NULL) {
		tst_report(0, "Context->gamma != NULL");
		return TST_FAILED;
	}

	if (c->axes_swap != 0 || c->x_swap != 0 || c->y_swap != 0) {
		tst_report(0, "Wrong default orientation %i %i %i",
		           c->axes_swap, c->x_swap, c->y_swap);
		return TST_FAILED;
	}

	/* access the pixel buffer */
	*(char*)GP_PIXEL_ADDR(c, 0, 0) = 0;
	*(char*)GP_PIXEL_ADDR(c, 100, 100) = 0;

	GP_ContextFree(c);

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Context Testsuite",
	.tests = {
		{.name = "Context Alloc Free", .tst_fn = Context_Alloc_Free,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
