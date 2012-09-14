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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>
#include <errno.h>

#include <core/GP_Context.h>
#include <loaders/GP_Loaders.h>

#include "tst_test.h"

static int test_PNG_Load_Save(void)
{
	GP_Context *img, *res;

	img = GP_ContextAlloc(100, 100, GP_PIXEL_RGB888);

	if (GP_SavePNG(img, "test.png", NULL)) {
		tst_report(0, "Failed to save PNG: %s", strerror(errno));
		return TST_FAILED;
	}

	res = GP_LoadPNG("test.png", NULL);

	if (res == NULL) {
		tst_report(0, "Failed to load PNG: %s", strerror(errno));
		return TST_FAILED;
	}

	GP_ContextFree(img);
	GP_ContextFree(res);

	return TST_SUCCESS;
}

static int test_JPG_Load_Save(void)
{
	GP_Context *img, *res;

	img = GP_ContextAlloc(100, 100, GP_PIXEL_RGB888);

	if (GP_SaveJPG(img, "test.jpg", NULL)) {
		tst_report(0, "Failed to save JPG: %s", strerror(errno));
		return TST_FAILED;
	}

	res = GP_LoadJPG("test.jpg", NULL);

	if (res == NULL) {
		tst_report(0, "Failed to load JPG: %s", strerror(errno));
		return TST_FAILED;
	}

	GP_ContextFree(img);
	GP_ContextFree(res);

	return TST_SUCCESS;
}

static int test_PNG_Load_fail(void)
{
	GP_Context *img;

	img = GP_LoadPNG("nonexistent.png", NULL);

	int saved_errno = errno;

	if (img != NULL) {
		tst_report(0, "Test succedded unexpectedly");
		return TST_FAILED;
	}

	if (saved_errno != ENOENT) {
		tst_report(0, "Expected errno = ENOENT, have %s",
		              strerror(errno));
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int test_JPG_Load_fail(void)
{
	GP_Context *img;

	img = GP_LoadJPG("nonexistent.png", NULL);

	int saved_errno = errno;

	if (img != NULL) {
		tst_report(0, "Test succedded unexpectedly");
		return TST_FAILED;
	}

	if (saved_errno != ENOENT) {
		tst_report(0, "Expected errno = ENOENT, have %s",
		              strerror(errno));
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int abort_callback(GP_ProgressCallback *self __attribute__((unused)))
{
	return 1;
}

/*
 * We test that a correct cleanup is done after aborting the image load from a
 * callback.
 */
static int test_PNG_Save_abort(void)
{
	GP_Context *img;

	img = GP_ContextAlloc(100, 100, GP_PIXEL_RGB888);

	GP_ProgressCallback callback = {.callback = abort_callback};

	if (GP_SavePNG(img, "test.png", &callback) == 0) {
		tst_report(0, "Failed to save PNG saving");
		return TST_FAILED;
	}
	
	int saved_errno = errno;
	
	if (saved_errno != ECANCELED) {
		tst_report(0, "Expected errno = ECANCELED, have %s",
		           strerror(saved_errno));
		return TST_FAILED;
	}

	GP_ContextFree(img);
	
	return TST_SUCCESS;
}

static int test_PNG_Load_abort(void)
{
	GP_Context *img;

	img = GP_ContextAlloc(100, 100, GP_PIXEL_RGB888);

	if (GP_SavePNG(img, "test.png", NULL)) {
		tst_report(0, "Failed to save PNG: %s", strerror(errno));
		return TST_FAILED;
	}

	GP_ContextFree(img);

	GP_ProgressCallback callback = {.callback = abort_callback};

	img = GP_LoadPNG("test.png", &callback);

	int saved_errno = errno;

	if (img != NULL) {
		tst_report(0, "Failed to abort PNG loading");
		return TST_FAILED;
	}

	if (saved_errno != ECANCELED) {
		tst_report(0, "Expected errno = ECANCELED, have %s",
		           strerror(saved_errno));
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

/*
 * PNG stress test, let it save and load 10MB image.
 */
static int test_PNG_stress(void)
{
	GP_Context *img;

	img = GP_ContextAlloc(2000, 2000, GP_PIXEL_RGB888);

	if (img == NULL) {
		tst_report(0, "GP_ContextAlloc failed");
		return TST_FAILED;
	}

	if (GP_SavePNG(img, "test.png", NULL)) {
		tst_report(0, "GP_SavePNG failed with: %s", strerror(errno));
		return TST_FAILED;
	}

	GP_ContextFree(img);

	img = GP_LoadPNG("test.png", NULL);
	
	if (img == NULL) {
		tst_report(0, "GP_LoadPNG failed with: %s", strerror(errno));
		return TST_FAILED;
	}

	GP_ContextFree(img);

	return TST_SUCCESS;
}

/*
 * Loaders test. Hammers the GP_LoadImage() interface with plenty of
 * unexpected filenames.
 */
struct file_testcase {
	const char *filename;
	int create;
	int expected_errno;
};

static struct file_testcase file_testcases[] = {
	{"a",     1, ENOSYS},
	{".a",    1, ENOSYS},
	{"a.",    1, ENOSYS},
	{".bc",   1, ENOSYS},
	{"bc",    1, ENOSYS},
	{"abc",   1, ENOSYS},
	{"png.",  1, ENOSYS},
	{"jpg.",  1, ENOSYS},
	{"gif.",  1, ENOSYS},
	{"jpeg.", 1, ENOSYS},
	
	{".jpg",    1, EIO},
	{"img.jpg", 1, EIO},
	{".png",    1, EIO},
	{"img.png", 1, EIO},
	{".gif",    1, EIO},
	{"img.gif", 1, EIO},
	{".pbm",    1, EIO},
	{".pgm",    1, EIO},
	{".ppm",    1, EIO},

	{"not_here.jpg", 0, ENOENT},

	{NULL, 0, 0}
};

static int test_Load(void)
{
	unsigned int i, fail = 0;

	/* Create empty files */
	for (i = 0; file_testcases[i].filename != NULL; i++) {
		
		if (file_testcases[i].create != 1)
			continue;

		FILE *f = fopen(file_testcases[i].filename, "w");

		if (f != NULL)
			fclose(f);
	}

	for (i = 0; file_testcases[i].filename != NULL; i++) {
		GP_Context *ret;
	
		errno = 0;

		ret = GP_LoadImage(file_testcases[i].filename, NULL);

		int saved_errno = errno;

		if (ret != NULL) {
			tst_report(0, "GP_LoadImage('%s') succeeded "
			              "unexpectedly", file_testcases[i].filename);
			fail++;
		}
		
		if (ret == NULL && file_testcases[i].expected_errno != errno) {
			tst_report(0, "Expected errno %i (%s) got %i (%s) on '%s'",
			              file_testcases[i].expected_errno,
				      strerror(file_testcases[i].expected_errno),
				      saved_errno,
				      strerror(saved_errno),
				      file_testcases[i].filename);
			fail++;
		}
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Image Loaders testsuite",
	.tests = {
		{.name = "PNG Load/Save", .tst_fn = test_PNG_Load_Save,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "JPG Load/Save", .tst_fn = test_JPG_Load_Save,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "PNG Load fail", .tst_fn = test_PNG_Load_fail,
		 .flags = TST_TMPDIR},
		{.name = "JPG Load fail", .tst_fn = test_JPG_Load_fail,
		 .flags = TST_TMPDIR},
		{.name = "PNG Load abort", .tst_fn = test_PNG_Load_abort,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "PNG Save abort", .tst_fn = test_PNG_Save_abort,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "PNG stress", .tst_fn = test_PNG_stress,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "Image Load", .tst_fn = test_Load,
		 .flags = TST_TMPDIR},
		{.name = NULL},
	}
};
