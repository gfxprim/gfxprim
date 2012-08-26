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
		{.name = NULL},
	}
};
