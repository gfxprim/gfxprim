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

  Very basic GP_Pixmap tests.

 */
#include <errno.h>

#include <core/GP_Pixmap.h>

#include "tst_test.h"

/*
 * Check that Pixmap is correctly allocated and freed
 */
static int Pixmap_Alloc_Free(void)
{
	GP_Pixmap *c;

	c = GP_PixmapAlloc(100, 200, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_msg("GP_PixmapAlloc() failed");
		return TST_FAILED;
	}

	/* Assert pixmap properties */
	if (c->bpp != 24) {
		tst_msg("Pixmap->bpp != 24 (== %i)", c->bpp);
		return TST_FAILED;
	}

	if (c->bytes_per_row != 3 * c->w) {
		tst_msg("Pixmap->bytes_per_row != %i (== %i)",
		        3 * c->w, c->bytes_per_row);
		return TST_FAILED;
	}

	if (c->w != 100) {
		tst_msg("Pixmap->w != 100 (== %i)", c->w);
		return TST_FAILED;
	}

	if (c->h != 200) {
		tst_msg("Pixmap->h != 200 (== %i)", c->h);
		return TST_FAILED;
	}

	if (c->offset != 0) {
		tst_msg("Pixmap->offset != 0");
		return TST_FAILED;
	}

	if (c->pixel_type != GP_PIXEL_RGB888) {
		tst_msg("Pixmap->pixel_type != GP_PIXEL_RGB888");
		return TST_FAILED;
	}

	if (c->gamma != NULL) {
		tst_msg("Pixmap->gamma != NULL");
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

	GP_PixmapFree(c);

	return TST_SUCCESS;
}

/*
 * Asserts that subpixmap structure is initialized correctly
 */
static int subpixmap_assert(const GP_Pixmap *c, const GP_Pixmap *sc,
                             GP_Size w, GP_Size h)
{
	if (c->bpp != sc->bpp) {
		tst_msg("Pixmap->bpp != SubPixmap->bpp");
		return TST_FAILED;
	}

	if (c->bytes_per_row != sc->bytes_per_row) {
		tst_msg("Pixmap->bytes_per_row != SubPixmap->bytes_per_row");
		return TST_FAILED;
	}

	if (sc->w != w) {
		tst_msg("SubPixmap->w != %u (== %i)", w, sc->w);
		return TST_FAILED;
	}

	if (sc->h != h) {
		tst_msg("SubPixmap->h != %u (== %i)", h, sc->h);
		return TST_FAILED;
	}

	if (sc->offset != 0) {
		tst_msg("SubPixmap->offset != 0");
		return TST_FAILED;
	}

	if (sc->pixel_type != GP_PIXEL_RGB888) {
		tst_msg("SubPixmap->pixel_type != GP_PIXEL_RGB888");
		return TST_FAILED;
	}

	if (sc->gamma != NULL) {
		tst_msg("SubPixmap->gamma != NULL");
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

static int SubPixmap_Alloc_Free(void)
{
	GP_Pixmap *c, *sc;
	int ret;

	c = GP_PixmapAlloc(300, 300, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_msg("GP_PixmapAlloc() failed");
		return TST_UNTESTED;
	}

	sc = GP_SubPixmapAlloc(c, 100, 100, 100, 100);

	if (sc == NULL) {
		GP_PixmapFree(c);
		return TST_FAILED;
	}

	ret = subpixmap_assert(c, sc, 100, 100);

	if (ret)
		return ret;

	GP_PixmapFree(c);
	GP_PixmapFree(sc);

	return TST_SUCCESS;
}

static int SubPixmap_Create(void)
{
	GP_Pixmap *c, sc;
	int ret;

	c = GP_PixmapAlloc(300, 300, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_msg("GP_PixmapAlloc() failed");
		return TST_UNTESTED;
	}

	GP_SubPixmap(c, &sc, 100, 100, 100, 100);

	ret = subpixmap_assert(c, &sc, 100, 100);

	if (ret)
		return ret;

	GP_PixmapFree(c);

	return TST_SUCCESS;
}

static int pixmap_zero_w(void)
{
	GP_Pixmap *c;

	c = GP_PixmapAlloc(0, 200, GP_PIXEL_G8);

	if (c != NULL) {
		tst_msg("Pixmap with zero width successfuly allocated");
		return TST_FAILED;
	}

	if (errno != EINVAL) {
		tst_msg("Expected errno set to EINVAL");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int pixmap_zero_h(void)
{
	GP_Pixmap *c;

	c = GP_PixmapAlloc(200, 0, GP_PIXEL_G8);

	if (c != NULL) {
		tst_msg("Pixmap with zero height successfuly allocated");
		return TST_FAILED;
	}

	if (errno != EINVAL) {
		tst_msg("Expected errno set to EINVAL");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int pixmap_invalid_pixeltype1(void)
{
	GP_Pixmap *c;

	c = GP_PixmapAlloc(100, 100, -1);

	if (c != NULL) {
		tst_msg("Pixmap with invalid pixel type (-1) succesfully allocated");
		return TST_FAILED;
	}

	if (errno != EINVAL) {
		tst_msg("Expected errno set to EINVAL");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int pixmap_invalid_pixeltype2(void)
{
	GP_Pixmap *c;

	c = GP_PixmapAlloc(100, 100, GP_PIXEL_MAX + 1000);

	if (c != NULL) {
		tst_msg("Pixmap with invalid pixel type (-1) succesfully allocated");
		return TST_FAILED;
	}

	if (errno != EINVAL) {
		tst_msg("Expected errno set to EINVAL");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Pixmap Testsuite",
	.tests = {
		{.name = "Pixmap Alloc Free", .tst_fn = Pixmap_Alloc_Free,
		 .flags = TST_CHECK_MALLOC},
		{.name = "SubPixmap Alloc Free",
		 .tst_fn = SubPixmap_Alloc_Free,
		 .flags = TST_CHECK_MALLOC},
		{.name = "SubPixmap Create",
		 .tst_fn = SubPixmap_Create},
		{.name = "Pixmap Create w = 0",
		 .tst_fn = pixmap_zero_w},
		{.name = "Pixmap Create h = 0",
		 .tst_fn = pixmap_zero_h},
		{.name = "Pixmap Create pixel_type = -1",
		 .tst_fn = pixmap_invalid_pixeltype1},
		{.name = "Pixmap Create invalid pixel_type",
		 .tst_fn = pixmap_invalid_pixeltype2},
		{.name = NULL},
	}
};
