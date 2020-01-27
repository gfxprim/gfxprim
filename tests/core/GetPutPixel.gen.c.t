@ include source.t
/*
 * GetPutPixel tests.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>

#include "tst_test.h"

static void fill_pixmap(gp_pixmap *c, gp_pixel p)
{
	gp_coord x, y;

	for (x = 0; x < (gp_coord)c->w; x++)
		for (y = 0; y < (gp_coord)c->h; y++)
			gp_putpixel(c, x, y, p);
}

static int check_filled(gp_pixmap *c)
{
	gp_coord x, y;
	gp_pixel p;

	p = gp_getpixel(c, 0, 0);

	for (x = 0; x < (gp_coord)c->w; x++)
		for (y = 0; y < (gp_coord)c->h; y++)
			if (p != gp_getpixel(c, x, y)) {
				tst_msg("Pixels different %i %i", x, y);
				return 1;
			}

	return 0;
}

static int try_pattern(gp_pixmap *c, gp_pixel p)
{
	fill_pixmap(c, p);

	tst_msg("Filling pattern 0x%x", p);

	if (check_filled(c))
		return 1;

	return 0;
}

@ for pt in pixeltypes:
@     if not pt.is_unknown():
static int GetPutPixel_{{ pt.name }}(void)
{
	gp_pixmap *c;
	int err = 0;

	c = gp_pixmap_alloc(100, 100, GP_PIXEL_{{ pt.name }});

	if (c == NULL) {
		tst_msg("gp_pixmap_alloc() failed");
		return TST_UNTESTED;
	}

	if (try_pattern(c, 0x55555555 & {{ 2 ** pt.pixelsize.size - 1}}U))
		err++;

	if (try_pattern(c, 0xaaaaaaaa & {{ 2 ** pt.pixelsize.size - 1}}U))
		err++;

	if (try_pattern(c, 0x0f0f0f0f & {{ 2 ** pt.pixelsize.size - 1}}U))
		err++;

	if (try_pattern(c, 0xf0f0f0f0 & {{ 2 ** pt.pixelsize.size - 1}}U))
		err++;

	gp_pixmap_free(c);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}
@ end

@ for pt in pixeltypes:
@     if not pt.is_unknown():
static int GetPutPixel_Clipping_{{ pt.name }}(void)
{
	gp_pixmap *c;

	c = gp_pixmap_alloc(100, 100, GP_PIXEL_{{ pt.name }});

	if (c == NULL) {
		tst_msg("gp_pixmap_alloc() failed");
		return TST_UNTESTED;
	}

	fill_pixmap(c, 0xffffffff);

	gp_coord x, y;
	int err = 0;

	for (x = -1000; x < 200; x++) {
		for (y = -1000; y < 200; y++) {
			if (x > 0 && x < 100 && y > 0 && y < 100)
				continue;

			/* Must be no-op */
			gp_putpixel(c, x, y, 0);

			/* Must return 0 */
			if (gp_getpixel(c, x, y) != 0) {
				tst_msg("gp_getpixel returned non-zero "
				        "at %i %i", x, y);
				err++;
			}
		}
	}

	gp_pixmap_free(c);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}
@ end

const struct tst_suite tst_suite = {
	.suite_name = "GetPutPixel Testsuite",
	.tests = {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
		{.name = "GetPutPixel {{ pt.name }}",
		 .tst_fn = GetPutPixel_{{ pt.name }}},
@ end

@ for pt in pixeltypes:
@     if not pt.is_unknown():
		{.name = "GetPutPixel Clipping {{ pt.name }}",
		 .tst_fn = GetPutPixel_Clipping_{{ pt.name }}},
@ end

		{.name = NULL}
	}
};
