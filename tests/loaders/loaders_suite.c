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
#include <sys/stat.h>

#include <core/GP_Context.h>
#include <loaders/GP_Loaders.h>

#include "tst_test.h"

enum fmt {
	PNG,
	JPG,
	GIF,
	BMP,
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
	case BMP:
		return "BMP";
	};

	return "INVALID";
}

static int save_img(enum fmt fmt, const GP_Context *img, const char *name)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "%s.%s", name, strfmt(fmt));

	switch (fmt) {
	case PNG:
		return GP_SavePNG(img, buf, NULL);
	case JPG:
		return GP_SaveJPG(img, buf, NULL);
	default:
		tst_report(0, "WARNING: trying to save %s", strfmt(fmt));
		exit(TST_INTERR);
	}
}

static GP_Context *load(enum fmt fmt, const char *name)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "%s.%s", name, strfmt(fmt));

	switch (fmt) {
	case PNG:
		return GP_LoadPNG(buf, NULL);
	case JPG:
		return GP_LoadJPG(buf, NULL);
	case GIF:
		return GP_LoadGIF(buf, NULL);
	case BMP:
		return GP_LoadBMP(buf, NULL);
	default:
		tst_report(0, "WARNING: trying to load %s", strfmt(fmt));
		exit(TST_INTERR);
	}
}

static int save_load(enum fmt fmt, GP_Size w, GP_Size h)
{
	GP_Context *img, *res;

	img = GP_ContextAlloc(w, h, GP_PIXEL_RGB888);
	
	if (img == NULL) {
		tst_report(0, "GP_ContextAlloc failed");
		return TST_INTERR;
	}

	int ret = save_img(fmt, img, "test");

	if (ret) {
		if (errno == ENOSYS) {
			tst_report(0, "Save %s: ENOSYS", strfmt(fmt));
			return TST_SKIPPED;
		}
		
		tst_report(0, "Failed to save %s: %s",
		           strfmt(fmt), strerror(errno));
		return TST_FAILED;
	}

	res = load(fmt, "test");

	if (res == NULL) {
		tst_report(0, "Failed to load %s: %s",
		           strfmt(fmt), strerror(errno));
		return TST_FAILED;
	}

	GP_ContextFree(img);
	GP_ContextFree(res);

	return TST_SUCCESS;
}

static int test_PNG_Save_Load(void)
{
	return save_load(PNG, 100, 100);
}

static int test_JPG_Save_Load(void)
{
	return save_load(JPG, 100, 100);
}

static int test_PNG_stress(void)
{
	return save_load(PNG, 2000, 2000);
}

static int test_JPG_stress(void)
{
	return save_load(JPG, 2000, 2000);
}

static int load_enoent(enum fmt fmt)
{
	GP_Context *img;
	
	img = load(fmt, "nonexistent");	
	
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

static int test_BMP_Load_ENOENT(void)
{
	return load_enoent(BMP);
}

static int load_eacces(enum fmt fmt)
{
	char buf[256];
	GP_Context *img;

	snprintf(buf, sizeof(buf), "test.%s", strfmt(fmt));
	
	FILE *f = fopen(buf, "w");

	if (f == NULL) {
		tst_report(0, "Failed to create file 'test'");
		return TST_INTERR;
	}

	fclose(f);

	if (chmod(buf, 200)) {
		tst_report(0, "chmod failed: %s", strerror(errno));
		return TST_INTERR;
	}

	img = load(fmt, "test");

	if (img != NULL) {
		tst_report(0, "Test succedded unexpectedly");
		return TST_FAILED;
	}
	
	if (errno == ENOSYS) {
		tst_report(0, "Load %s: ENOSYS", strfmt(fmt));
		return TST_SKIPPED;
	}

	if (errno != EACCES) {
		tst_report(0, "Expected errno = EACCES, have %s",
		              strerror(errno));
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int test_PNG_Load_EACCES(void)
{
	return load_eacces(PNG);
}

static int test_JPG_Load_EACCES(void)
{
	return load_eacces(JPG);
}

static int test_GIF_Load_EACCES(void)
{
	return load_eacces(GIF);
}

static int test_BMP_Load_EACCES(void)
{
	return load_eacces(BMP);
}

static int load_eio(enum fmt fmt)
{
	char buf[256];
	GP_Context *img;

	snprintf(buf, sizeof(buf), "test.%s", strfmt(fmt));
	
	FILE *f = fopen(buf, "w");

	if (f == NULL) {
		tst_report(0, "Failed to create file 'test'");
		return TST_INTERR;
	}

	fclose(f);

	img = load(fmt, "test");

	if (img != NULL) {
		tst_report(0, "Test succedded unexpectedly");
		return TST_FAILED;
	}
	
	if (errno == ENOSYS) {
		tst_report(0, "Load %s: ENOSYS", strfmt(fmt));
		return TST_SKIPPED;
	}

	if (errno != EIO) {
		tst_report(0, "Expected errno = EIO, have %s",
		              strerror(errno));
		return TST_FAILED;
	}
	
	return TST_SUCCESS;
}

static int test_PNG_Load_EIO(void)
{
	return load_eio(PNG);
}

static int test_JPG_Load_EIO(void)
{
	return load_eio(JPG);
}

static int test_GIF_Load_EIO(void)
{
	return load_eio(GIF);
}

static int test_BMP_Load_EIO(void)
{
	return load_eio(BMP);
}

/*
 * We test that a correct cleanup is done after aborting the image load from a
 * callback.
 */
static int abort_callback(GP_ProgressCallback *self __attribute__((unused)))
{
	return 1;
}

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

	//TODO: EACCES

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

		if (file_testcases[i].expected_errno != ENOSYS && errno == ENOSYS) {
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
		/* Correct errno tests */
		{.name = "PNG Load ENOENT", .tst_fn = test_PNG_Load_ENOENT,
		 .flags = TST_TMPDIR},
		{.name = "JPG Load ENOENT", .tst_fn = test_JPG_Load_ENOENT,
		 .flags = TST_TMPDIR},
		{.name = "GIF Load ENOENT", .tst_fn = test_GIF_Load_ENOENT,
		 .flags = TST_TMPDIR},
		{.name = "BMP Load ENOENT", .tst_fn = test_BMP_Load_ENOENT,
		 .flags = TST_TMPDIR},

		{.name = "PNG Load EACCES", .tst_fn = test_PNG_Load_EACCES,
		 .flags = TST_TMPDIR},
		{.name = "JPG Load EACCES", .tst_fn = test_JPG_Load_EACCES,
		 .flags = TST_TMPDIR},
		{.name = "GIF Load EACCES", .tst_fn = test_GIF_Load_EACCES,
		 .flags = TST_TMPDIR},
		{.name = "BMP Load EACCES", .tst_fn = test_BMP_Load_EACCES,
		 .flags = TST_TMPDIR},
		
		{.name = "PNG Load EIO", .tst_fn = test_PNG_Load_EIO,
		 .flags = TST_TMPDIR},
		{.name = "JPG Load EIO", .tst_fn = test_JPG_Load_EIO,
		 .flags = TST_TMPDIR},
		{.name = "GIF Load EIO", .tst_fn = test_GIF_Load_EIO,
		 .flags = TST_TMPDIR},
		{.name = "BMP Load EIO", .tst_fn = test_BMP_Load_EIO,
		 .flags = TST_TMPDIR},
		
		/* Generic GP_LoadImage test */
		{.name = "Image Load", .tst_fn = test_Load,
		 .flags = TST_TMPDIR},
		
		/* Callback abort tests */
		{.name = "PNG Load abort", .tst_fn = test_PNG_Load_abort,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "PNG Save abort", .tst_fn = test_PNG_Save_abort,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		/* Basic Save Load tests */
		{.name = "PNG Save Load", .tst_fn = test_PNG_Save_Load,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "JPG Save Load", .tst_fn = test_JPG_Save_Load,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		/* Stress Save Load tests */
		{.name = "PNG Stress", .tst_fn = test_PNG_stress,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = "JPG Stress", .tst_fn = test_JPG_stress,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		{.name = NULL},
	}
};
