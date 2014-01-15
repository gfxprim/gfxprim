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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef TESTS_LOADER_H
#define TESTS_LOADER_H

#include "loaders/GP_IO.h"

struct testcase {
	GP_Size w;
	GP_Size h;
	GP_Pixel pix;
	char *path;
};

static int test_check(struct testcase *test, GP_Context *img)
{
	unsigned int x, y, err = 0;

	if (img->w != test->w || img->h != test->h) {
		tst_msg("Invalid image size have %ux%u expected %ux%u",
		        img->w, img->h, test->w, test->h);
		GP_ContextFree(img);
		return TST_FAILED;
	}

	for (x = 0; x < img->w; x++) {
		for (y = 0; y < img->h; y++) {

			GP_Pixel pix = GP_GetPixel(img, x, y);

			if (pix != test->pix) {
				if (err < 5)
					tst_msg("%08x instead of %08x (%ux%u)",
					        pix, test->pix, x, y);
				err++;
			}
		}
	}

	if (err > 5)
		tst_msg("And %u errors...", err);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}

static int test_read(struct testcase *test)
{
	GP_Context *img;
	GP_IO *io;
	int err;

	io = GP_IOMem(test->path, strlen(test->path), NULL);

	if (!io) {
		tst_msg("Failed to initialize memory IO: %s", strerror(errno));
		return TST_UNTESTED;
	}

	img = READ(io, NULL);

	if (!img) {
		switch (errno) {
		case ENOSYS:
			tst_msg("Not Implemented");
			err = TST_SKIPPED;
			goto out;
		default:
			tst_msg("Got %s", strerror(errno));
			err = TST_FAILED;
			goto out;
		}
	}

	err = test_check(test, img);

	GP_ContextFree(img);
out:
	GP_IOClose(io);
	return err;
}

# ifdef LOAD

static int test_load(struct testcase *test)
{
	GP_Context *img;
	int err;

	errno = 0;

	img = LOAD(test->path, NULL);

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

	err = test_check(test, img);

	GP_ContextFree(img);

	return err;
}


static int test_load_fail(const char *path)
{
	GP_Context *img;

	errno = 0;

	img = LOAD(path, NULL);

	if (img != NULL) {
		tst_msg("Succeeded unexpectedly");
		GP_ContextFree(img);
		return TST_FAILED;
	}

	switch (errno) {
	case ENOSYS:
		tst_msg("Not Implemented");
		return TST_SKIPPED;
	case 0:
		tst_msg("Load failed and errno == 0");
		return TST_FAILED;
	default:
		tst_msg("Got %s", strerror(errno));
		return TST_SUCCESS;
	}
}

# endif /* LOAD */


# if defined(SAVE) && defined(LOAD)

/*
 * Saves and loads image using the SAVE and LOAD functions
 * and compares the results.
 */
struct testcase_save_load {
	GP_PixelType pixel_type;
	GP_Size w, h;
};

static int test_save_load(struct testcase_save_load *test)
{
	GP_Context *img, *img2;
	unsigned int x, y;

	img = GP_ContextAlloc(test->w, test->h, test->pixel_type);

	if (img == NULL) {
		tst_msg("Failed to allocate context %ux%u %s",
		        test->w, test->h, GP_PixelTypeName(test->pixel_type));
		return TST_FAILED;
	}

	for (x = 0; x < img->w; x++)
		for (y = 0; y < img->w; y++)
			GP_PutPixel(img, x, y, 0);

	errno = 0;

	if (SAVE(img, "testfile", NULL)) {
		if (errno == ENOSYS) {
			tst_msg("Not implemented");
			return TST_SKIPPED;
		}

		tst_msg("Failed to save context %ux%u %s: %s",
		        test->w, test->h, GP_PixelTypeName(test->pixel_type),
			strerror(errno));
		return TST_FAILED;
	}


	img2 = LOAD("testfile", NULL);

	if (img2 == NULL) {
		switch (errno) {
		case ENOSYS:
			tst_msg("Not Implemented");
			GP_ContextFree(img);
			return TST_SKIPPED;
		default:
			tst_msg("Got %s", strerror(errno));
			GP_ContextFree(img);
			return TST_FAILED;
		}
	}

	if (img->w != img2->w || img->h != img2->h) {
		tst_msg("Source size %ux%u and loaded size %ux%u differs",
		        img->w, img->h, img2->w, img2->h);
		GP_ContextFree(img);
		GP_ContextFree(img2);
		return TST_FAILED;
	}

	if (img->pixel_type != img2->pixel_type) {
		GP_ContextFree(img);
		GP_ContextFree(img2);
		tst_msg("Source pixel type %s and loaded type %s differs",
			GP_PixelTypeName(img->pixel_type),
			GP_PixelTypeName(img2->pixel_type));
		return TST_FAILED;
	}

	if (GP_GetPixel(img2, 0, 0) != 0) {
		tst_msg("Pixel value is wrong %x", GP_GetPixel(img2, 0, 0));
		GP_ContextFree(img);
		GP_ContextFree(img2);
		return TST_FAILED;
	}

	GP_ContextFree(img);
	GP_ContextFree(img2);

	return TST_SUCCESS;
}

# endif /* SAVE && LOAD */

#endif /* TESTS_LOADER_H */
