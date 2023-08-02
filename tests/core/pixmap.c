// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Very basic gp_pixmap tests.

 */
#include <errno.h>
#include <string.h>
#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include "tst_test.h"

/*
 * Check that Pixmap is correctly allocated and freed
 */
static int Pixmap_Alloc_Free(void)
{
	gp_pixmap *c;

	c = gp_pixmap_alloc(100, 200, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_msg("gp_pixmap_alloc() failed");
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

	gp_pixmap_free(c);

	return TST_PASSED;
}

/*
 * Asserts that subpixmap structure is initialized correctly
 */
static int subpixmap_assert(const gp_pixmap *c, const gp_pixmap *sc,
                             gp_size w, gp_size h)
{
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
	gp_pixmap *c, *sc;
	int ret;

	c = gp_pixmap_alloc(300, 300, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_msg("gp_pixmap_alloc() failed");
		return TST_UNTESTED;
	}

	sc = gp_sub_pixmap_alloc(c, 100, 100, 100, 100);

	if (sc == NULL) {
		gp_pixmap_free(c);
		return TST_FAILED;
	}

	ret = subpixmap_assert(c, sc, 100, 100);

	if (ret)
		return ret;

	gp_pixmap_free(c);
	gp_pixmap_free(sc);

	return TST_PASSED;
}

static int SubPixmap_Create(void)
{
	gp_pixmap *c, sc;
	int ret;

	c = gp_pixmap_alloc(300, 300, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_msg("gp_pixmap_alloc() failed");
		return TST_UNTESTED;
	}

	gp_sub_pixmap(c, &sc, 100, 100, 100, 100);

	ret = subpixmap_assert(c, &sc, 100, 100);

	if (ret)
		return ret;

	gp_pixmap_free(c);

	return TST_PASSED;
}

static int pixmap_zero_w(void)
{
	gp_pixmap *c;

	c = gp_pixmap_alloc(0, 200, GP_PIXEL_G8);

	if (c != NULL) {
		tst_msg("Pixmap with zero width successfuly allocated");
		return TST_FAILED;
	}

	if (errno != EINVAL) {
		tst_msg("Expected errno set to EINVAL");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int pixmap_zero_h(void)
{
	gp_pixmap *c;

	c = gp_pixmap_alloc(200, 0, GP_PIXEL_G8);

	if (c != NULL) {
		tst_msg("Pixmap with zero height successfuly allocated");
		return TST_FAILED;
	}

	if (errno != EINVAL) {
		tst_msg("Expected errno set to EINVAL");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int pixmap_invalid_pixeltype1(void)
{
	gp_pixmap *c;

	c = gp_pixmap_alloc(100, 100, -1);

	if (c != NULL) {
		tst_msg("Pixmap with invalid pixel type (-1) succesfully allocated");
		return TST_FAILED;
	}

	if (errno != EINVAL) {
		tst_msg("Expected errno set to EINVAL");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int pixmap_invalid_pixeltype2(void)
{
	gp_pixmap *c;

	c = gp_pixmap_alloc(100, 100, GP_PIXEL_MAX + 1000);

	if (c != NULL) {
		tst_msg("Pixmap with invalid pixel type (-1) succesfully allocated");
		return TST_FAILED;
	}

	if (errno != EINVAL) {
		tst_msg("Expected errno set to EINVAL");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int pixmap_from_data(void)
{
	gp_pixmap *p;
	unsigned char *data = malloc(100);
	unsigned int i, j;

	memset(data, 127, 100);

	p = gp_pixmap_from_data(10, 10, GP_PIXEL_G8, data, GP_PIXMAP_FREE_PIXELS);
	if (!p) {
		tst_msg("Malloc failed :(");
		return TST_FAILED;
	}

	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			if (gp_getpixel(p, i, j) != 127) {
				tst_msg("Wrong data in pixmap at %u, %u!", i, j);
				return TST_FAILED;
			}
		}
	}

	gp_pixmap_free(p);
	return TST_PASSED;
}

static int pixmap_from_static_data(void)
{
	gp_pixmap *p;
	unsigned char data[] = {0, 0, 0,
	                        0, 1, 0,
	                        0, 0, 0};

	p = gp_pixmap_from_data(3, 3, GP_PIXEL_G8, data, 0);

	if (gp_getpixel(p, 1, 1) != 1) {
		tst_msg("Wrong data at 1, 1");
		return TST_FAILED;
	}

	gp_pixmap_free(p);
	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "Pixmap Testsuite",
	.tests = {
		{.name = "Pixmap Alloc Free", .tst_fn = Pixmap_Alloc_Free,
		 .flags = TST_CHECK_MALLOC},
		{.name = "SubPixmap Alloc Free",
		 .tst_fn = SubPixmap_Alloc_Free,
		 .flags = TST_CHECK_MALLOC},
		{.name = "pixmap from data",
		 .tst_fn = pixmap_from_data,
		 .flags = TST_CHECK_MALLOC},
		{.name = "pixmap from static data",
		 .tst_fn = pixmap_from_static_data,
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
