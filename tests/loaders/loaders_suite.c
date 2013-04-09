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
		tst_err("Trying to save %s image", strfmt(fmt));
		exit(TST_UNTESTED);
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
		tst_err("Trying to load %s image", strfmt(fmt));
		exit(TST_UNTESTED);
	}
}

static int save_load(enum fmt fmt, GP_Size w, GP_Size h)
{
	GP_Context *img, *res;

	img = GP_ContextAlloc(w, h, GP_PIXEL_RGB888);
	
	if (img == NULL) {
		tst_warn("GP_ContextAlloc failed");
		return TST_UNTESTED;
	}

	int ret = save_img(fmt, img, "test");

	if (ret) {
		if (errno == ENOSYS) {
			tst_msg("Save %s: ENOSYS", strfmt(fmt));
			return TST_SKIPPED;
		}
		
		tst_msg("Failed to save %s: %s",
		           strfmt(fmt), strerror(errno));
		return TST_FAILED;
	}

	res = load(fmt, "test");

	if (res == NULL) {
		tst_msg("Failed to load %s: %s",
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
		tst_msg("Test succedded unexpectedly");
		return TST_FAILED;
	}
	
	if (errno == ENOSYS) {
		tst_msg("Load %s: ENOSYS", strfmt(fmt));
		return TST_SKIPPED;
	}

	if (errno != ENOENT) {
		tst_msg("Expected errno = ENOENT, have %s", strerror(errno));
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
		tst_err("Failed to create file 'test': %s", strerror(errno));
		return TST_UNTESTED;
	}

	fclose(f);

	if (chmod(buf, 200)) {
		tst_err("chmod failed: %s", strerror(errno));
		return TST_UNTESTED;
	}

	img = load(fmt, "test");

	if (img != NULL) {
		tst_msg("Test succedded unexpectedly");
		return TST_FAILED;
	}
	
	if (errno == ENOSYS) {
		tst_msg("Load %s: ENOSYS", strfmt(fmt));
		return TST_SKIPPED;
	}

	if (errno != EACCES) {
		tst_msg("Expected errno = EACCES, have %s",
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
		tst_err("Failed to create file 'test': %s", strerror(errno));
		return TST_UNTESTED;
	}

	fclose(f);

	img = load(fmt, "test");

	if (img != NULL) {
		tst_msg("Test succedded unexpectedly");
		return TST_FAILED;
	}
	
	if (errno == ENOSYS) {
		tst_msg("Load %s: ENOSYS", strfmt(fmt));
		return TST_SKIPPED;
	}

	if (errno != EIO) {
		tst_msg("Expected errno = EIO, have %s",
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
		tst_msg("Failed to save PNG saving");
		return TST_FAILED;
	}
	
	if (errno == ENOSYS) {
		tst_msg("Load PNG: ENOSYS");
		return TST_SKIPPED;
	}

	if (errno != ECANCELED) {
		tst_msg("Expected errno = ECANCELED, have %s",
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
			tst_msg("Save PNG: ENOSYS");
			return TST_SKIPPED;
		}
		
		tst_msg("Failed to save PNG: %s", strerror(errno));
		return TST_FAILED;
	}

	GP_ContextFree(img);

	GP_ProgressCallback callback = {.callback = abort_callback};

	img = GP_LoadPNG("test.png", &callback);

	int saved_errno = errno;

	if (img != NULL) {
		tst_msg("Failed to abort PNG loading");
		return TST_FAILED;
	}

	if (saved_errno != ECANCELED) {
		tst_msg("Expected errno = ECANCELED, have %s",
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

enum file_flags {
	FILE_CREATE = 0x01,
	FILE_FILL = 0x02,
};

static struct file_testcase file_testcases[] = {
	
	/*
	 * Should fail the file signature based loader
	 * as the signature could have been loaded but
	 * the file format is unknown.
	 */
	{"a",     FILE_CREATE | FILE_FILL, ENOSYS},
	{".a",    FILE_CREATE | FILE_FILL, ENOSYS},
	{"a.",    FILE_CREATE | FILE_FILL, ENOSYS},
	{".bc",   FILE_CREATE | FILE_FILL, ENOSYS},
	{"bc",    FILE_CREATE | FILE_FILL, ENOSYS},
	{"abc",   FILE_CREATE | FILE_FILL, ENOSYS},

	/*
	 * Should fail the corresponding loader and
	 * then fail the signature based loader too.
	 */
	{"wrong.png",  FILE_CREATE | FILE_FILL, ENOSYS},
	{"wrong.jpg",  FILE_CREATE | FILE_FILL, ENOSYS},
	{"wrong.gif",  FILE_CREATE | FILE_FILL, ENOSYS},
	{"wrong.jpeg", FILE_CREATE | FILE_FILL, ENOSYS},
	{"wrong.bmp",  FILE_CREATE | FILE_FILL, ENOSYS},
	{"wrong.psp",  FILE_CREATE | FILE_FILL, ENOSYS},
	{"wrong.tif",  FILE_CREATE | FILE_FILL, ENOSYS},
	{"wrong.tiff", FILE_CREATE | FILE_FILL, ENOSYS},

	/* 
	 * Should start signature-based loader and
	 * fail it to read start of the file.
	 */
	{"b",          FILE_CREATE, EIO},
	{".b",         FILE_CREATE, EIO},
	{"b.",         FILE_CREATE, EIO},
	{".dc",        FILE_CREATE, EIO},
	{"dc",         FILE_CREATE, EIO},
	{"cba",        FILE_CREATE, EIO},
	
	/*
	 * Dtto but for hits the extension based
	 * loader first and fail to read image header
	 * in the particular loader.
	 */
	{"empty.jpg",  FILE_CREATE, EIO},
	{"empty.png",  FILE_CREATE, EIO},
	{"empty.gif",  FILE_CREATE, EIO},
	{"empty.jpeg", FILE_CREATE, EIO},
	{"empty.bmp",  FILE_CREATE, EIO},
	{"empty.psp",  FILE_CREATE, EIO},
	{"empty.tif",  FILE_CREATE, EIO},
	{"empty.tiff", FILE_CREATE, EIO},

	{".jpg",    FILE_CREATE, EIO},
	{".png",    FILE_CREATE, EIO},
	{".gif",    FILE_CREATE, EIO},
	{".jpeg",    FILE_CREATE, EIO},
	{".bmp",    FILE_CREATE, EIO},
	{".psp",    FILE_CREATE, EIO},
	{".tiff",    FILE_CREATE, EIO},

	{"not_here.jpg", 0, ENOENT},

	//TODO: EACCES

	{NULL, 0, 0}
};

static int test_Load(void)
{
	unsigned int i, fail = 0, success = 0;

	/* Create empty files */
	for (i = 0; file_testcases[i].filename != NULL; i++) {

		if (!(file_testcases[i].create & FILE_CREATE))
			continue;

		FILE *f = fopen(file_testcases[i].filename, "wb");

		if (f != NULL) {
			char buf[128] = {0};

			if (file_testcases[i].create & FILE_FILL) {
				if (fwrite(buf, sizeof(buf), 1, f) != 1)
					tst_msg("Failed to write to '%s'",
				        	file_testcases[i].filename);
			
			}

			fclose(f);
		} else {
			tst_msg("Failed to open file '%s' for writing",
			        file_testcases[i].filename);
		}
	}

	for (i = 0; file_testcases[i].filename != NULL; i++) {
		GP_Context *ret;
	
		errno = 0;

		ret = GP_LoadImage(file_testcases[i].filename, NULL);

		int saved_errno = errno;

		if (ret != NULL) {
			tst_msg("GP_LoadImage('%s') succeeded unexpectedly",
			        file_testcases[i].filename);
			fail++;
			continue;
		}

		if (file_testcases[i].expected_errno != ENOSYS && errno == ENOSYS) {
			tst_msg("Load Image '%s': ENOSYS",
			        file_testcases[i].filename);
			continue;
		}
		
		if (file_testcases[i].expected_errno != saved_errno) {
			tst_msg("Expected errno %i (%s) got %i (%s) on '%s'",
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

static int test_load_BMP(const char *path)
{
	GP_Context *img;

	img = GP_LoadBMP(path, NULL);

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

	GP_ContextFree(img);

	return TST_SUCCESS;
}

/* Basic loading tests */

static int test_load_BMP_1bpp_1x1(void)
{
	return test_load_BMP("1bpp-1x1.bmp");
}

static int test_load_BMP_4bpp_1x1(void)
{
	return test_load_BMP("4bpp-1x1.bmp");
}

static int test_load_BMP_8bpp_1x1(void)
{
	return test_load_BMP("8bpp-1x1.bmp");
}

static int test_load_BMP_24bpp_1x1(void)
{
	return test_load_BMP("24bpp-1x1.bmp");
}

static int test_load_BMP_32bpp_1x1(void)
{
	return test_load_BMP("32bpp-1x1.bmp");
}

static int test_load_BMP_555_1x1(void)
{
	return test_load_BMP("555-1x1.bmp");
}

static int test_load_BMP_565_1x1(void)
{
	return test_load_BMP("565-1x1.bmp");
}

static int test_load_BMP_8bpp_1x64000(void)
{
	return test_load_BMP("8bpp-1x64000.bmp");
}

static int test_load_JPEG(const char *path)
{
	GP_Context *img;

	img = GP_LoadJPG(path, NULL);

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
	GP_ContextFree(img);

	return TST_SUCCESS;
}

static int test_load_JPEG_100x100(void)
{
	return test_load_JPEG("100x100-red.jpeg");
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
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
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

		/* BPM loader tests */
		{.name = "BMP Load 1bpp 1x1",
		 .tst_fn = test_load_BMP_1bpp_1x1,
		 .res_path = "data/bmp/bitmaps/valid/1bpp-1x1.bmp",
		 .flags = TST_TMPDIR},
		
		{.name = "BMP Load 4bpp 1x1",
		 .tst_fn = test_load_BMP_4bpp_1x1,
		 .res_path = "data/bmp/bitmaps/valid/4bpp-1x1.bmp",
		 .flags = TST_TMPDIR},
		
		{.name = "BMP Load 8bpp 1x1",
		 .tst_fn = test_load_BMP_8bpp_1x1,
		 .res_path = "data/bmp/bitmaps/valid/8bpp-1x1.bmp",
		 .flags = TST_TMPDIR},
		
		{.name = "BMP 24bpp 1x1",
		 .tst_fn = test_load_BMP_24bpp_1x1,
		 .res_path = "data/bmp/bitmaps/valid/24bpp-1x1.bmp",
		 .flags = TST_TMPDIR},
		
		{.name = "BMP 32bpp 1x1",
		 .tst_fn = test_load_BMP_32bpp_1x1,
		 .res_path = "data/bmp/bitmaps/valid/32bpp-1x1.bmp",
		 .flags = TST_TMPDIR},
		
		{.name = "BMP 555 1x1",
		 .tst_fn = test_load_BMP_555_1x1,
		 .res_path = "data/bmp/bitmaps/valid/555-1x1.bmp",
		 .flags = TST_TMPDIR},
		
		{.name = "BMP 565 1x1",
		 .tst_fn = test_load_BMP_565_1x1,
		 .res_path = "data/bmp/bitmaps/valid/565-1x1.bmp",
		 .flags = TST_TMPDIR},
		
		{.name = "BMP 8bpp 1x64000",
		 .tst_fn = test_load_BMP_8bpp_1x64000,
		 .res_path = "data/bmp/bitmaps/valid/8bpp-1x64000.bmp",
		 .flags = TST_TMPDIR},

		/* JPEG loader tests */
		{.name = "JPEG 100x100", .tst_fn = test_load_JPEG_100x100,
		 .res_path = "data/jpeg/valid/100x100-red.jpeg",
		 .flags = TST_TMPDIR},

		{.name = NULL},
	}
};
