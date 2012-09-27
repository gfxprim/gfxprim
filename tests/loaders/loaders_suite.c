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

enum fmt {
	PNG,
	JPG,
	GIF,
};

static const char *strfmt(enum fmt fmt)
{
	switch (fmt) {
	case PNG:
		return "PNG";
	case JPG:
		return "JPG";
	case GIF:
		return "GIF";
	};

	return "INVALID";
}

static int load_save(enum fmt fmt, GP_Size w, GP_Size h)
{
	GP_Context *img, *res = NULL;

	img = GP_ContextAlloc(w, h, GP_PIXEL_RGB888);
	
	if (img == NULL) {
		tst_report(0, "GP_ContextAlloc failed");
		return TST_FAILED;
	}

	int ret = 1;

	switch (fmt) {
	case PNG:
		ret = GP_SavePNG(img, "test.png", NULL);
	break;
	case JPG:
		ret = GP_SaveJPG(img, "test.jpg", NULL);
	break;
	default:
		return TST_INTERR;
	}

	if (ret) {
		if (errno == ENOSYS) {
			tst_report(0, "Save %s: ENOSYS", strfmt(fmt));
			return TST_SKIPPED;
		}
		
		tst_report(0, "Failed to save %s: %s",
		           strfmt(fmt), strerror(errno));
		return TST_FAILED;
	}

	switch (fmt) {
	case PNG:
		res = GP_LoadPNG("test.png", NULL);
	break;
	case JPG:
		res = GP_LoadJPG("test.jpg", NULL);
	break;
	default:
		return TST_INTERR;
	}

	if (res == NULL) {
		tst_report(0, "Failed to load %s: %s",
		           strfmt(fmt), strerror(errno));
		return TST_FAILED;
	}

	GP_ContextFree(img);
	GP_ContextFree(res);

	return TST_SUCCESS;
}

static int test_PNG_Load_Save(void)
{
	return load_save(PNG, 100, 100);
}

static int test_JPG_Load_Save(void)
{
	return load_save(JPG, 100, 100);
}

static int test_PNG_stress(void)
{
	return load_save(PNG, 2000, 2000);
}

static int test_JPG_stress(void)
{
	return load_save(JPG, 2000, 2000);
}

static int load_enoent(enum fmt fmt)
{
	GP_Context *img = NULL;

	switch (fmt) {
	case PNG:
		img = GP_LoadPNG("nonexistent.png", NULL);
	break;
	case JPG:
		img = GP_LoadJPG("nonexistent.jpg", NULL);
	break;
	case GIF:
		img = GP_LoadGIF("nonexistent.gif", NULL);
	break;
	}
	
	if (img != NULL) {
		tst_report(0, "Test succedded unexpectedly");
		return TST_FAILED;
	}
	
	if (errno == ENOSYS) {
		tst_report(0, "Load %s: ENOSYS", strfmt(fmt));
		return TST_SKIPPED;
	}

	if (errno != ENOENT) {
		tst_report(0, "Expected errno = ENOENT, have %s",
		              strerror(errno));
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int test_PNG_Load_ENOENT(void)
{
	return load_enoent(PNG);
}

static int test_JPG_Load_ENOENT(void)
{
	return load_enoent(JPG);
}

static int test_GIF_Load_ENOENT(void)
{
	return load_enoent(GIF);
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
	
	if (errno == ENOSYS) {
		tst_report(0, "Load PNG: ENOSYS");
		return TST_SKIPPED;
	}

	if (errno != ECANCELED) {
		tst_report(0, "Expected errno = ECANCELED, have %s",
		           strerror(errno));
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
		
		if (errno == ENOSYS) {
			tst_report(0, "Save PNG: ENOSYS");
			return TST_SKIPPED;
		}
		
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
	{"bmp.", 1, ENOSYS},
	
	{".jpg",    1, EIO},
	{"img.jpg", 1, EIO},
	{".png",    1, EIO},
	{"img.png", 1, EIO},
	{".gif",    1, EIO},
	{"img.gif", 1, EIO},
	{".bmp",    1, EIO},
	{"img.bmp", 1, EIO},
	{".pbm",    1, EIO},
	{".pgm",    1, EIO},
	{".ppm",    1, EIO},

	{"not_here.jpg", 0, ENOENT},

	//TODO: EPERM

	{NULL, 0, 0}
};

static int test_Load(void)
{
	unsigned int i, fail = 0, success = 0;

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
			continue;
		}

		if (errno == ENOSYS) {
			tst_report(0, "Load Image '%s': ENOSYS",
			           file_testcases[i].filename);
			continue;
		}
		
		if (file_testcases[i].expected_errno != errno) {
			tst_report(0, "Expected errno %i (%s) got %i (%s) on '%s'",
			              file_testcases[i].expected_errno,
				      strerror(file_testcases[i].expected_errno),
				      saved_errno,
				      strerror(saved_errno),
				      file_testcases[i].filename);
			fail++;
			continue;
		}

		success++;
	}

	if (fail)
		return TST_FAILED;

	if (success)
		return TST_SUCCESS;

	return TST_SKIPPED;
}

const struct tst_suite tst_suite = {
	.suite_name = "Image Loaders testsuite",
	.tests = {
		{.name = "PNG Load/Save", .tst_fn = test_PNG_Load_Save,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "JPG Load/Save", .tst_fn = test_JPG_Load_Save,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "PNG Load ENOENT", .tst_fn = test_PNG_Load_ENOENT,
		 .flags = TST_TMPDIR},
		{.name = "JPG Load ENOENT", .tst_fn = test_JPG_Load_ENOENT,
		 .flags = TST_TMPDIR},
		{.name = "GIF Load ENOENT", .tst_fn = test_GIF_Load_ENOENT,
		 .flags = TST_TMPDIR},
		{.name = "PNG Load abort", .tst_fn = test_PNG_Load_abort,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "PNG Save abort", .tst_fn = test_PNG_Save_abort,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "Image Load", .tst_fn = test_Load,
		 .flags = TST_TMPDIR},
		{.name = "PNG stress", .tst_fn = test_PNG_stress,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "JPG stress", .tst_fn = test_JPG_stress,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = NULL},
	}
};
