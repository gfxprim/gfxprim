// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

struct test {
	gp_size w, h;
	const char *path;
};

static int test_load(struct test *test)
{
	gp_container *zip = gp_open_zip(test->path);
	gp_pixmap *img;
	int ret = TST_PASSED;

	if (!zip) {
		if (errno == ENOSYS) {
			tst_msg("Zlib support not compiled in?");
			return TST_UNTESTED;
		}

		tst_msg("Failed to open zip");
		return TST_FAILED;
	}

	img = gp_container_load(zip, NULL);

	if (!img) {
		if (!errno) {
			tst_msg("No images found, support not compiled in?");
			return TST_UNTESTED;
		}

		tst_msg("Failed to load image: %s", strerror(errno));
		return TST_FAILED;
	}

	if (img->w != test->w || img->h != test->h) {
		tst_msg("Image has wrong size, expected %ux%u have %ux%u",
		        test->w, test->h, img->w, img->h);
		ret = TST_FAILED;
	}

	gp_pixmap_free(img);
	gp_container_close(zip);

	return ret;
}

struct test jpeg_deflated = {
	.w = 100,
	.h = 100,
	.path = "jpeg_deflated.zip"
};

struct test jpeg_stored = {
	.w = 100,
	.h = 100,
	.path = "jpeg_stored.zip"
};

struct test mixed_content = {
	.w = 100,
	.h = 100,
	.path = "mixed_content.zip"
};

/*
 * Test ZIP with no images, we expect to get NULL with errno set to zero
 */
static int no_images(const char *path)
{
	gp_container *zip = gp_open_zip(path);
	gp_pixmap *img;
	int ret = TST_PASSED;

	if (!zip) {
		if (errno == ENOSYS) {
			tst_msg("Zlib support not compiled in?");
			return TST_UNTESTED;
		}

		tst_msg("Failed to open zip");
		return TST_FAILED;
	}

	img = gp_container_load(zip, NULL);
	if (img) {
		tst_msg("Loaded image from zip without images");
		ret = TST_FAILED;
	}

	if (errno) {
		tst_msg("Get errno %d (%s)", errno, strerror(errno));
		ret = TST_FAILED;
	}

	gp_container_close(zip);

	return ret;
}

const struct tst_suite tst_suite = {
	.suite_name = "ZIP",
	.tests = {
		{.name = "Load JPEG deflated in ZIP",
		 .tst_fn = test_load,
		 .res_path = "data/zip/valid/jpeg_deflated.zip",
		 .data = &jpeg_deflated,
		 .flags = TST_TMPDIR | TST_MALLOC_CANARIES},

		{.name = "Load JPEG stored in ZIP",
		 .tst_fn = test_load,
		 .res_path = "data/zip/valid/jpeg_stored.zip",
		 .data = &jpeg_stored,
		 .flags = TST_TMPDIR | TST_MALLOC_CANARIES},

		{.name = "Load JPEG is second file in ZIP",
		 .tst_fn = test_load,
		 .res_path = "data/zip/valid/mixed_content.zip",
		 .data = &mixed_content,
		 .flags = TST_TMPDIR | TST_MALLOC_CANARIES},

		{.name = "Load ZIP with no images",
		 .tst_fn = no_images,
		 .res_path = "data/zip/valid/no_images.zip",
		 .data = "no_images.zip",
		 .flags = TST_TMPDIR | TST_MALLOC_CANARIES},

		{.name = NULL},
	}
};
