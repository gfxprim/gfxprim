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
		tst_msg("GP_ContextAlloc() failed");
		return TST_FAILED;
	}

	/* Assert context properties */
	if (c->bpp != 24) {
		tst_msg("Context->bpp != 24 (== %i)", c->bpp);
		return TST_FAILED;
	}

	if (c->bytes_per_row != 3 * c->w) {
		tst_msg("Context->bytes_per_row != %i (== %i)",
		        3 * c->w, c->bytes_per_row);
		return TST_FAILED;
	}

	if (c->w != 100) {
		tst_msg("Context->w != 100 (== %i)", c->w);
		return TST_FAILED;
	}
	
	if (c->h != 200) {
		tst_msg("Context->h != 200 (== %i)", c->h);
		return TST_FAILED;
	}
	
	if (c->offset != 0) {
		tst_msg("Context->offset != 0");
		return TST_FAILED;
	}

	if (c->pixel_type != GP_PIXEL_RGB888) {
		tst_msg("Context->pixel_type != GP_PIXEL_RGB888");
		return TST_FAILED;
	}

	if (c->gamma != NULL) {
		tst_msg("Context->gamma != NULL");
		return TST_FAILED;
	}

	if (c->axes_swap != 0 || c->x_swap != 0 || c->y_swap != 0) {
		tst_msg("Wrong default orientation %i %i %i",
		        c->axes_swap, c->x_swap, c->y_swap);
		return TST_FAILED;
	}

	/* access the pixel buffer */
	*(char*)GP_PIXEL_ADDR(c, 0, 0) = 0;
	*(char*)GP_PIXEL_ADDR(c, 100, 100) = 0;

	GP_ContextFree(c);

	return TST_SUCCESS;
}

/*
 * Asserts that subcontext structure is initialized correctly
 */
static int subcontext_assert(const GP_Context *c, const GP_Context *sc,
                             GP_Size w, GP_Size h)
{
	if (c->bpp != sc->bpp) {
		tst_msg("Context->bpp != SubContext->bpp");
		return TST_FAILED;
	}

	if (c->bytes_per_row != sc->bytes_per_row) {
		tst_msg("Context->bytes_per_row != SubContext->bytes_per_row");
		return TST_FAILED;
	}
	
	if (sc->w != w) {
		tst_msg("SubContext->w != %u (== %i)", w, sc->w);
		return TST_FAILED;
	}
	
	if (sc->h != h) {
		tst_msg("SubContext->h != %u (== %i)", h, sc->h);
		return TST_FAILED;
	}
	
	if (sc->offset != 0) {
		tst_msg("SubContext->offset != 0");
		return TST_FAILED;
	}

	if (sc->pixel_type != GP_PIXEL_RGB888) {
		tst_msg("SubContext->pixel_type != GP_PIXEL_RGB888");
		return TST_FAILED;
	}

	if (sc->gamma != NULL) {
		tst_msg("SubContext->gamma != NULL");
		return TST_FAILED;
	}

	if (sc->axes_swap != 0 || sc->x_swap != 0 || sc->y_swap != 0) {
		tst_msg("Wrong default orientation %i %i %i",
		        sc->axes_swap, sc->x_swap, sc->y_swap);
		return TST_FAILED;
	}
	
	/* access the start and end of the pixel buffer */
	*(char*)GP_PIXEL_ADDR(sc, 0, 0) = 0;
	*(char*)GP_PIXEL_ADDR(sc, sc->w - 1, sc->h - 1) = 0;

	return 0;
}

static int SubContext_Alloc_Free(void)
{
	GP_Context *c, *sc;
	int ret;

	c = GP_ContextAlloc(300, 300, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_msg("GP_ContextAlloc() failed");
		return TST_UNTESTED;
	}

	sc = GP_SubContextAlloc(c, 100, 100, 100, 100);

	if (sc == NULL) {
		GP_ContextFree(c);
		return TST_FAILED;
	}

	ret = subcontext_assert(c, sc, 100, 100);

	if (ret)
		return ret;

	GP_ContextFree(c);
	GP_ContextFree(sc);

	return TST_SUCCESS;
}

static int SubContext_Create(void)
{
	GP_Context *c, sc;
	int ret;

	c = GP_ContextAlloc(300, 300, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_msg("GP_ContextAlloc() failed");
		return TST_UNTESTED;
	}

	GP_SubContext(c, &sc, 100, 100, 100, 100);
	
	ret = subcontext_assert(c, &sc, 100, 100);

	if (ret)
		return ret;

	GP_ContextFree(c);

	return TST_SUCCESS;
}


const struct tst_suite tst_suite = {
	.suite_name = "Context Testsuite",
	.tests = {
		{.name = "Context Alloc Free", .tst_fn = Context_Alloc_Free,
		 .flags = TST_CHECK_MALLOC},
		{.name = "SubContext Alloc Free",
		 .tst_fn = SubContext_Alloc_Free,
		 .flags = TST_CHECK_MALLOC},
		{.name = "SubContext Create",
		 .tst_fn = SubContext_Create},
		{.name = NULL},
	}
};
