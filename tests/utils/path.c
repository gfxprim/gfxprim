// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <utils/gp_utils.h>
#include "tst_test.h"

static int compose_path_01(void)
{
	char *res;

	res = gp_compose_path("/tmp", "dir", "foo/bar/", "file");
	if (!res) {
		tst_msg("Allocation failed?!");
		return TST_FAILED;
	}

	if (strcmp(res, "/tmp/dir/foo/bar/file")) {
		tst_msg("Wrong path '%s'", res);
		return TST_FAILED;
	}

	free(res);

	return TST_PASSED;
}

static int compose_path_02(void)
{
	char *res;

	res = gp_compose_path("/tmp");
	if (!res) {
		tst_msg("Allocation failed?!");
		return TST_FAILED;
	}

	if (strcmp(res, "/tmp")) {
		tst_msg("Wrong path '%s'", res);
		return TST_FAILED;
	}

	free(res);

	return TST_PASSED;
}

static int compose_path_03(void)
{
	char *res;

	res = gp_compose_path("/tmp/", "dir/bar//", "/file");
	if (!res) {
		tst_msg("Allocation failed?!");
		return TST_FAILED;
	}

	if (strcmp(res, "/tmp/dir/bar/file")) {
		tst_msg("Wrong path '%s'", res);
		return TST_FAILED;
	}

	free(res);

	return TST_PASSED;
}

static int compose_path_04(void)
{
	char *res;

	res = gp_compose_path("/tmp/", "//", "/file");
	if (!res) {
		tst_msg("Allocation failed?!");
		return TST_FAILED;
	}

	if (strcmp(res, "/tmp/file")) {
		tst_msg("Wrong path '%s'", res);
		return TST_FAILED;
	}

	free(res);

	return TST_PASSED;
}

static int mkpath_01(void)
{
	int ret;

	ret = gp_mkpath("./", "foo/bar//dir", 0, 0700);
	if (ret) {
		tst_msg("mkpath failed: %s", strerror(errno));
		return TST_FAILED;
	}

	int fd = open("./foo/bar/dir/file", O_CREAT | O_WRONLY, 0644);
	if (!fd) {
		tst_msg("open: %s", strerror(errno));
		return TST_FAILED;
	}

	close(fd);

	return TST_PASSED;
}

static int mkpath_02(void)
{
	int ret;

	ret = gp_mkpath(NULL, "./foo/bar//dir/file", GP_MKPATH_HAS_FILE, 0700);
	if (ret) {
		tst_msg("mkpath failed: %s", strerror(errno));
		return TST_FAILED;
	}

	int fd = open("./foo/bar/dir/file", O_EXCL | O_CREAT | O_WRONLY, 0644);
	if (!fd) {
		tst_msg("open: %s", strerror(errno));
		return TST_FAILED;
	}

	close(fd);

	return TST_PASSED;
}

static int mkpath_03(void)
{
	int ret;

	ret = gp_mkpath(NULL, "./foo/bar/dir/", 0, 0700);
	if (ret) {
		tst_msg("mkpath failed: %s", strerror(errno));
		return TST_FAILED;
	}

	ret = gp_mkpath(NULL, "./foo/bar/dir/", 0, 0700);
	if (ret) {
		tst_msg("mkpath failed: %s", strerror(errno));
		return TST_FAILED;
	}

	int fd = open("./foo/bar/dir/", O_DIRECTORY);
	if (!fd) {
		tst_msg("Failed to open newly created directory!");
		return TST_FAILED;
	}

	close(fd);

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "path testsuite",
	.tests = {
		{.name = "compose path 01",
		 .tst_fn = compose_path_01,
		 .flags = TST_CHECK_MALLOC},

		{.name = "compose path 02",
		 .tst_fn = compose_path_02,
		 .flags = TST_CHECK_MALLOC},

		{.name = "compose path 03",
		 .tst_fn = compose_path_03,
		 .flags = TST_CHECK_MALLOC},

		{.name = "compose path 04",
		 .tst_fn = compose_path_04,
		 .flags = TST_CHECK_MALLOC},

		{.name = "mkpath 01",
		 .tst_fn = mkpath_01,
		 .flags = TST_CHECK_MALLOC | TST_TMPDIR},

		{.name = "mkpath 02",
		 .tst_fn = mkpath_02,
		 .flags = TST_CHECK_MALLOC | TST_TMPDIR},

		{.name = "mkpath 03",
		 .tst_fn = mkpath_03,
		 .flags = TST_CHECK_MALLOC | TST_TMPDIR},

		{},
	}
};
