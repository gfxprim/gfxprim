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

static int test_load_gif(const char *path)
{
	gp_pixmap *img;

	errno = 0;

	img = gp_load_gif(path, NULL);

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

	gp_pixmap_free(img);

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "gif",
	.tests = {
		{.name = "gif Load 100x100",
		 .tst_fn = test_load_gif,
		 .res_path = "data/gif/valid/100x100-white.gif",
		 .data = "100x100-white.gif",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
