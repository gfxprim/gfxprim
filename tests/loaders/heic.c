// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

static int test_load_heic(const char *path)
{
	gp_pixmap *img;

	errno = 0;

	img = gp_load_heif(path, NULL);
	if (!img) {
		switch (errno) {
		case ENOSYS:
			tst_msg("Not Implemented");
			return TST_SKIPPED;
		default:
			tst_msg("Got %s", strerror(errno));
			return TST_FAILED;
		}
	}

	gp_pixmap_free(img);

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "HEIC",
	.tests = {
		{.name = "HEIC Load",
		 .tst_fn = test_load_heic,
		 .res_path = "data/heic/valid/100x100-red.heic",
		 .data = "100x100-red.heic",
		 .flags = TST_TMPDIR},

		{.name = "HEIC Load",
		 .tst_fn = test_load_heic,
		 .res_path = "data/heic/valid/chef-with-trumpet.heic",
		 .data = "chef-with-trumpet.heic",
		 .flags = TST_TMPDIR},

		{.name = NULL},
	}
};
