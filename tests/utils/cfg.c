// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <utils/gp_utils.h>
#include "tst_test.h"

static int cfg_path(void)
{
	char *cfg_path;

	setenv("HOME", "/tmp/", 1);

	cfg_path = gp_app_cfg_path("app_name", "cfg.txt");
	if (!cfg_path) {
		tst_msg("gp_app_cfg_path() returned NULL");
		return TST_FAILED;
	}

	if (strcmp(cfg_path, "/tmp/.config/app_name/cfg.txt")) {
		tst_msg("Got wrong path '%s'!", cfg_path);
		return TST_FAILED;
	}

	free(cfg_path);

	return TST_SUCCESS;
}

static int cfg_mkpath(void)
{
	char buf[2048];
	int fd;

	setenv("HOME", getcwd(buf, sizeof(buf)), 1);

	if (gp_app_cfg_mkpath("app_name")) {
		tst_msg("gp_app_cfg_mkdirs() failed: %s", strerror(errno));
		return TST_FAILED;
	}

	fd = open(".config/app_name", O_DIRECTORY);
	if (fd < 0) {
		tst_msg("Failed to open newly created directory");
		return TST_FAILED;
	}

	close(fd);

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "app cfg testsuite",
	.tests = {
		{.name = "cfg path",
		 .tst_fn = cfg_path},

		{.name = "cfg mkpath",
		 .tst_fn = cfg_mkpath,
		 .flags = TST_TMPDIR},

		{.name = NULL},
	}
};
