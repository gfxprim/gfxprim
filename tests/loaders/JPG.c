// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

static int test_load_jpg(const char *path)
{
	gp_pixmap *img;

	errno = 0;

	img = gp_load_jpg(path, NULL);

	if (img == NULL) {
		switch (errno) {
		case ENOSYS:
			tst_msg("Not Implemented");
			return TST_SKIPPED;
		default:
			tst_msg("Got %s", strerror(errno));
			return TST_FAILED;
		}
	}

	/*
	 * TODO: check correct data.
	 */

	gp_pixmap_free(img);

	return TST_SUCCESS;
}

static int test_save_jpg(gp_pixel_type pixel_type)
{
	gp_pixmap *pixmap;
	int ret;

	pixmap = gp_pixmap_alloc(100, 100, pixel_type);

	if (pixmap == NULL) {
		tst_msg("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	errno = 0;

	ret = gp_save_jpg(pixmap, "/dev/null", NULL);

	if (ret == 0) {
		tst_msg("Saved successfully");
		gp_pixmap_free(pixmap);
		return TST_SUCCESS;
	}

	switch (errno) {
	case ENOSYS:
		tst_msg("Not Implemented");
		gp_pixmap_free(pixmap);
		return TST_SKIPPED;
	default:
		tst_msg("Failed and errno is not ENOSYS (%i)", errno);
		gp_pixmap_free(pixmap);
		return TST_FAILED;
	}
}

const struct tst_suite tst_suite = {
	.suite_name = "JPEG",
	.tests = {
		/* JPEG loader tests */
		{.name = "JPEG Load 100x100 CMYK",
		 .tst_fn = test_load_jpg,
		 .res_path = "data/jpeg/valid/100x100-cmyk-red.jpeg",
		 .data = "100x100-cmyk-red.jpeg",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Load 100x100 RGB",
		 .tst_fn = test_load_jpg,
		 .res_path = "data/jpeg/valid/100x100-red.jpeg",
		 .data = "100x100-red.jpeg",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Load 100x100 G8",
		 .tst_fn = test_load_jpg,
		 .res_path = "data/jpeg/valid/100x100-grayscale-white.jpeg",
		 .data = "100x100-grayscale-white.jpeg",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Load 100x100 G8",
		 .tst_fn = test_load_jpg,
		 .res_path = "data/jpeg/valid/100x100-grayscale-black.jpeg",
		 .data = "100x100-grayscale-black.jpeg",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		/* JPEG save tests */
		{.name = "JPEG Save 100x100 G8",
		 .tst_fn = test_save_jpg,
		 .data = (void*)GP_PIXEL_G8,
		 .flags = TST_CHECK_MALLOC},

		{.name = "JPEG Save 100x100 RGB888",
		 .tst_fn = test_save_jpg,
		 .data = (void*)GP_PIXEL_RGB888,
		 .flags = TST_CHECK_MALLOC},

		{.name = "JPEG Save 100x100 BGR888",
		 .tst_fn = test_save_jpg,
		 .data = (void*)GP_PIXEL_BGR888,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
