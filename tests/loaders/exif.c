// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <loaders/gp_exif.h>

#include "tst_test.h"

struct testcase {
	const char *path;
};

static int test_load_Exif(struct testcase *test)
{
	gp_storage *data = gp_storage_create();
	gp_io *io;
	int ret = TST_PASSED;

	if (!data) {
		tst_msg("Failed to create DataStorage");
		return TST_UNTESTED;
	}

	io = gp_io_file(test->path, GP_IO_RDONLY);

	if (!io) {
		tst_msg("Failed to open IO %s", test->path);
		gp_storage_destroy(data);
		return TST_UNTESTED;
	}

	if (gp_read_exif(io, data)) {
		tst_msg("Failed to load Exif");
		ret = TST_FAILED;
		goto end;
	}

end:
	gp_storage_destroy(data);
	gp_io_close(io);
	return ret;
}

static struct testcase sample001 = {
	.path = "sample001.exif",
};

static struct testcase sample002 = {
	.path = "sample002.exif",
};

const struct tst_suite tst_suite = {
	.suite_name = "Exif",
	.tests = {
		{.name = "Exif sample001",
		 .tst_fn = test_load_Exif,
		 .res_path = "data/exif/sample001.exif",
		 .data = &sample001,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "Exif sample002",
		 .tst_fn = test_load_Exif,
		 .res_path = "data/exif/sample002.exif",
		 .data = &sample002,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
