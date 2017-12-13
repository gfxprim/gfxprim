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

#include <loaders/GP_IO.h>

struct testcase {
	gp_size w;
	gp_size h;
	gp_pixel pix;
	char *path;
};

static int test_check(struct testcase *test, gp_pixmap *img)
{
	unsigned int x, y, err = 0;

	if (img->w != test->w || img->h != test->h) {
		tst_msg("Invalid image size have %ux%u expected %ux%u",
		        img->w, img->h, test->w, test->h);
		gp_pixmap_free(img);
		return TST_FAILED;
	}

	for (x = 0; x < img->w; x++) {
		for (y = 0; y < img->h; y++) {

			gp_pixel pix = gp_getpixel(img, x, y);

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
	gp_pixmap *img;
	gp_io *io;
	int err;

	io = gp_io_mem(test->path, strlen(test->path), NULL);

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

	gp_pixmap_free(img);
out:
	gp_io_close(io);
	return err;
}

# ifdef LOAD

static int test_load(struct testcase *test)
{
	gp_pixmap *img;
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

	gp_pixmap_free(img);

	return err;
}


static int test_load_fail(const char *path)
{
	gp_pixmap *img;

	errno = 0;

	img = LOAD(path, NULL);

	if (img != NULL) {
		tst_msg("Succeeded unexpectedly");
		gp_pixmap_free(img);
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
	gp_pixel_type pixel_type;
	gp_size w, h;
};

static int test_save_load(struct testcase_save_load *test)
{
	gp_pixmap *img, *img2;
	unsigned int x, y;

	img = gp_pixmap_alloc(test->w, test->h, test->pixel_type);

	if (img == NULL) {
		tst_msg("Failed to allocate pixmap %ux%u %s",
		        test->w, test->h, gp_pixel_type_name(test->pixel_type));
		return TST_FAILED;
	}

	for (x = 0; x < img->w; x++)
		for (y = 0; y < img->w; y++)
			gp_putpixel(img, x, y, 0);

	errno = 0;

	if (SAVE(img, "testfile", NULL)) {
		if (errno == ENOSYS) {
			tst_msg("Not implemented");
			return TST_SKIPPED;
		}

		tst_msg("Failed to save pixmap %ux%u %s: %s",
		        test->w, test->h, gp_pixel_type_name(test->pixel_type),
			strerror(errno));
		return TST_FAILED;
	}


	img2 = LOAD("testfile", NULL);

	if (img2 == NULL) {
		switch (errno) {
		case ENOSYS:
			tst_msg("Not Implemented");
			gp_pixmap_free(img);
			return TST_SKIPPED;
		default:
			tst_msg("Got %s", strerror(errno));
			gp_pixmap_free(img);
			return TST_FAILED;
		}
	}

	if (img->w != img2->w || img->h != img2->h) {
		tst_msg("Source size %ux%u and loaded size %ux%u differs",
		        img->w, img->h, img2->w, img2->h);
		gp_pixmap_free(img);
		gp_pixmap_free(img2);
		return TST_FAILED;
	}

	if (img->pixel_type != img2->pixel_type) {
		gp_pixmap_free(img);
		gp_pixmap_free(img2);
		tst_msg("Source pixel type %s and loaded type %s differs",
			gp_pixel_type_name(img->pixel_type),
			gp_pixel_type_name(img2->pixel_type));
		return TST_FAILED;
	}

	if (gp_getpixel(img2, 0, 0) != 0) {
		tst_msg("Pixel value is wrong %x", gp_getpixel(img2, 0, 0));
		gp_pixmap_free(img);
		gp_pixmap_free(img2);
		return TST_FAILED;
	}

	gp_pixmap_free(img);
	gp_pixmap_free(img2);

	return TST_SUCCESS;
}

# endif /* SAVE && LOAD */

#endif /* TESTS_LOADER_H */
