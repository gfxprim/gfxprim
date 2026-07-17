// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

static int test_load_jpg(const char *path)
{
	gp_pixmap *img;

	errno = 0;

	img = gp_load_jpg(path, NULL);

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

	gp_pixmap_free(img);

	return TST_PASSED;
}

struct corr_testcase {
	const char *path;
	int null_storage;
	gp_correction_type corr_type;
	float gamma;
};

static int test_load_jpg_correction(struct corr_testcase *test)
{
	gp_storage *storage = NULL;
	gp_pixmap *img = NULL;
	gp_gamma_table *lin;
	gp_io *io;
	int err, ret = TST_PASSED;

	if (!test->null_storage) {
		storage = gp_storage_create();
		if (!storage) {
			tst_msg("Failed to create storage");
			return TST_UNTESTED;
		}
	}

	io = gp_io_file(test->path, GP_IO_RDONLY);
	if (!io) {
		tst_msg("Failed to open '%s'", test->path);
		gp_storage_destroy(storage);
		return TST_UNTESTED;
	}

	gp_image_info image_info = {.meta_data = storage};

	err = gp_read_jpg_ex(io, &img, &image_info, NULL);
	if (err) {
		switch (errno) {
		case ENOSYS:
			tst_msg("Not Implemented");
			return TST_SKIPPED;
		default:
			tst_msg("Got %s", strerror(errno));
			return TST_FAILED;
		}
	}

	if (gp_pixmap_w(img) != 10 || gp_pixmap_h(img) != 10) {
		tst_msg("Wrong pixmap size %ux%u expected 10x10",
		        gp_pixmap_w(img), gp_pixmap_h(img));
		ret = TST_FAILED;
		goto end;
	}

	if (!img->gamma || !img->gamma->lin[0]) {
		tst_msg("Loaded pixmap has no correction set");
		ret = TST_FAILED;
		goto end;
	}

	lin = img->gamma->lin[0];

	if (lin->corr_type != test->corr_type) {
		tst_msg("Wrong correction '%s' expected '%s'",
		        gp_correction_type_name(lin->corr_type),
		        gp_correction_type_name(test->corr_type));
		ret = TST_FAILED;
		goto end;
	}

	if (test->corr_type == GP_CORRECTION_TYPE_GAMMA &&
	    fabsf(lin->gamma - test->gamma) > 0.001f) {
		tst_msg("Wrong gamma %f expected %f", lin->gamma, test->gamma);
		ret = TST_FAILED;
	}

end:
	gp_pixmap_free(img);
	gp_io_close(io);
	gp_storage_destroy(storage);
	return ret;
}

static struct corr_testcase cs_missing_nogamma_ii = {
	.path = "cs-missing_nogamma_ii.jpg",
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_missing_nogamma_mm = {
	.path = "cs-missing_nogamma_mm.jpg",
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_missing_gamma22_ii = {
	.path = "cs-missing_gamma22_ii.jpg",
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_missing_gamma22_mm = {
	.path = "cs-missing_gamma22_mm.jpg",
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_srgb_nogamma_ii = {
	.path = "cs-srgb_nogamma_ii.jpg",
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_srgb_nogamma_mm = {
	.path = "cs-srgb_nogamma_mm.jpg",
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_srgb_gamma22_ii = {
	.path = "cs-srgb_gamma22_ii.jpg",
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_srgb_gamma22_mm = {
	.path = "cs-srgb_gamma22_mm.jpg",
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_adobe_nogamma_ii = {
	.path = "cs-adobe_nogamma_ii.jpg",
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_adobe_nogamma_mm = {
	.path = "cs-adobe_nogamma_mm.jpg",
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_adobe_gamma22_ii = {
	.path = "cs-adobe_gamma22_ii.jpg",
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_adobe_gamma22_mm = {
	.path = "cs-adobe_gamma22_mm.jpg",
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_uncal_nogamma_ii = {
	.path = "cs-uncalibrated_nogamma_ii.jpg",
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_uncal_nogamma_mm = {
	.path = "cs-uncalibrated_nogamma_mm.jpg",
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_uncal_gamma22_ii = {
	.path = "cs-uncalibrated_gamma22_ii.jpg",
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_uncal_gamma22_mm = {
	.path = "cs-uncalibrated_gamma22_mm.jpg",
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_missing_nogamma_ii_null = {
	.path = "cs-missing_nogamma_ii.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_missing_nogamma_mm_null = {
	.path = "cs-missing_nogamma_mm.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_missing_gamma22_ii_null = {
	.path = "cs-missing_gamma22_ii.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_missing_gamma22_mm_null = {
	.path = "cs-missing_gamma22_mm.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_srgb_nogamma_ii_null = {
	.path = "cs-srgb_nogamma_ii.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_srgb_nogamma_mm_null = {
	.path = "cs-srgb_nogamma_mm.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_srgb_gamma22_ii_null = {
	.path = "cs-srgb_gamma22_ii.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_srgb_gamma22_mm_null = {
	.path = "cs-srgb_gamma22_mm.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_adobe_nogamma_ii_null = {
	.path = "cs-adobe_nogamma_ii.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_adobe_nogamma_mm_null = {
	.path = "cs-adobe_nogamma_mm.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_adobe_gamma22_ii_null = {
	.path = "cs-adobe_gamma22_ii.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_adobe_gamma22_mm_null = {
	.path = "cs-adobe_gamma22_mm.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_uncal_nogamma_ii_null = {
	.path = "cs-uncalibrated_nogamma_ii.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_uncal_nogamma_mm_null = {
	.path = "cs-uncalibrated_nogamma_mm.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_uncal_gamma22_ii_null = {
	.path = "cs-uncalibrated_gamma22_ii.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_uncal_gamma22_mm_null = {
	.path = "cs-uncalibrated_gamma22_mm.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_GAMMA,
	.gamma = 2.2,
};

static struct corr_testcase cs_uncal_gamma0den_ii_null = {
	.path = "cs-uncalibrated_gamma0den_ii.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};

static struct corr_testcase cs_uncal_gamma0den_mm_null = {
	.path = "cs-uncalibrated_gamma0den_mm.jpg",
	.null_storage = 1,
	.corr_type = GP_CORRECTION_TYPE_SRGB,
};


static int test_save_jpg(gp_pixel_type pixel_type)
{
	gp_pixmap *pixmap;
	int ret;

	pixmap = gp_pixmap_alloc(100, 100, pixel_type);

	if (pixmap == NULL) {
		tst_msg("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	errno = 0;

	ret = gp_save_jpg(pixmap, "/dev/null", NULL);

	if (ret == 0) {
		tst_msg("Saved successfully");
		gp_pixmap_free(pixmap);
		return TST_PASSED;
	}

	switch (errno) {
	case ENOSYS:
		tst_msg("Not Implemented");
		gp_pixmap_free(pixmap);
		return TST_SKIPPED;
	default:
		tst_msg("Failed and errno is not ENOSYS (%i)", errno);
		gp_pixmap_free(pixmap);
		return TST_FAILED;
	}
}

const struct tst_suite tst_suite = {
	.suite_name = "JPEG",
	.tests = {
		/* JPEG loader tests */
		{.name = "JPEG Load 100x100 CMYK",
		 .tst_fn = test_load_jpg,
		 .res_path = "data/jpeg/valid/100x100-cmyk-red.jpeg",
		 .data = "100x100-cmyk-red.jpeg",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Load 100x100 RGB",
		 .tst_fn = test_load_jpg,
		 .res_path = "data/jpeg/valid/100x100-red.jpeg",
		 .data = "100x100-red.jpeg",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Load 100x100 G8",
		 .tst_fn = test_load_jpg,
		 .res_path = "data/jpeg/valid/100x100-grayscale-white.jpeg",
		 .data = "100x100-grayscale-white.jpeg",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Load 100x100 G8",
		 .tst_fn = test_load_jpg,
		 .res_path = "data/jpeg/valid/100x100-grayscale-black.jpeg",
		 .data = "100x100-grayscale-black.jpeg",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		/* Exif ColorSpace/Gamma -> pixmap correction tests */
		{.name = "JPEG Exif no ColorSpace no Gamma II",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-missing_nogamma_ii.jpg",
		 .data = &cs_missing_nogamma_ii,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif no ColorSpace no Gamma MM",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-missing_nogamma_mm.jpg",
		 .data = &cs_missing_nogamma_mm,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif no ColorSpace Gamma 2.2 II",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-missing_gamma22_ii.jpg",
		 .data = &cs_missing_gamma22_ii,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif no ColorSpace Gamma 2.2 MM",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-missing_gamma22_mm.jpg",
		 .data = &cs_missing_gamma22_mm,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif sRGB no Gamma II",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-srgb_nogamma_ii.jpg",
		 .data = &cs_srgb_nogamma_ii,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif sRGB no Gamma MM",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-srgb_nogamma_mm.jpg",
		 .data = &cs_srgb_nogamma_mm,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif sRGB Gamma 2.2 II",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-srgb_gamma22_ii.jpg",
		 .data = &cs_srgb_gamma22_ii,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif sRGB Gamma 2.2 MM",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-srgb_gamma22_mm.jpg",
		 .data = &cs_srgb_gamma22_mm,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif invalid ColorSpace no Gamma II",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-adobe_nogamma_ii.jpg",
		 .data = &cs_adobe_nogamma_ii,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif invalid ColorSpace no Gamma MM",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-adobe_nogamma_mm.jpg",
		 .data = &cs_adobe_nogamma_mm,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif invalid ColorSpace Gamma 2.2 II",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-adobe_gamma22_ii.jpg",
		 .data = &cs_adobe_gamma22_ii,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif invalid ColorSpace Gamma 2.2 MM",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-adobe_gamma22_mm.jpg",
		 .data = &cs_adobe_gamma22_mm,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif Uncalibrated no Gamma II",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-uncalibrated_nogamma_ii.jpg",
		 .data = &cs_uncal_nogamma_ii,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif Uncalibrated no Gamma MM",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-uncalibrated_nogamma_mm.jpg",
		 .data = &cs_uncal_nogamma_mm,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif Uncalibrated Gamma 2.2 II",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-uncalibrated_gamma22_ii.jpg",
		 .data = &cs_uncal_gamma22_ii,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif Uncalibrated Gamma 2.2 MM",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-uncalibrated_gamma22_mm.jpg",
		 .data = &cs_uncal_gamma22_mm,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif no ColorSpace no Gamma II NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-missing_nogamma_ii.jpg",
		 .data = &cs_missing_nogamma_ii_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif no ColorSpace no Gamma MM NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-missing_nogamma_mm.jpg",
		 .data = &cs_missing_nogamma_mm_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif no ColorSpace Gamma 2.2 II NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-missing_gamma22_ii.jpg",
		 .data = &cs_missing_gamma22_ii_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif no ColorSpace Gamma 2.2 MM NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-missing_gamma22_mm.jpg",
		 .data = &cs_missing_gamma22_mm_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif sRGB no Gamma II NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-srgb_nogamma_ii.jpg",
		 .data = &cs_srgb_nogamma_ii_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif sRGB no Gamma MM NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-srgb_nogamma_mm.jpg",
		 .data = &cs_srgb_nogamma_mm_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif sRGB Gamma 2.2 II NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-srgb_gamma22_ii.jpg",
		 .data = &cs_srgb_gamma22_ii_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif sRGB Gamma 2.2 MM NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-srgb_gamma22_mm.jpg",
		 .data = &cs_srgb_gamma22_mm_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif invalid ColorSpace no Gamma II NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-adobe_nogamma_ii.jpg",
		 .data = &cs_adobe_nogamma_ii_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif invalid ColorSpace no Gamma MM NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-adobe_nogamma_mm.jpg",
		 .data = &cs_adobe_nogamma_mm_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif invalid ColorSpace Gamma 2.2 II NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-adobe_gamma22_ii.jpg",
		 .data = &cs_adobe_gamma22_ii_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif invalid ColorSpace Gamma 2.2 MM NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-adobe_gamma22_mm.jpg",
		 .data = &cs_adobe_gamma22_mm_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif Uncalibrated no Gamma II NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-uncalibrated_nogamma_ii.jpg",
		 .data = &cs_uncal_nogamma_ii_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif Uncalibrated no Gamma MM NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-uncalibrated_nogamma_mm.jpg",
		 .data = &cs_uncal_nogamma_mm_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif Uncalibrated Gamma 2.2 II NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-uncalibrated_gamma22_ii.jpg",
		 .data = &cs_uncal_gamma22_ii_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif Uncalibrated Gamma 2.2 MM NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-uncalibrated_gamma22_mm.jpg",
		 .data = &cs_uncal_gamma22_mm_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif Uncalibrated Gamma 22/0 II NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-uncalibrated_gamma0den_ii.jpg",
		 .data = &cs_uncal_gamma0den_ii_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "JPEG Exif Uncalibrated Gamma 22/0 MM NULL storage",
		 .tst_fn = test_load_jpg_correction,
		 .res_path = "data/jpeg/exif/cs-uncalibrated_gamma0den_mm.jpg",
		 .data = &cs_uncal_gamma0den_mm_null,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		/* JPEG save tests */
		{.name = "JPEG Save 100x100 G8",
		 .tst_fn = test_save_jpg,
		 .data = (void*)GP_PIXEL_G8,
		 .flags = TST_CHECK_MALLOC},

		{.name = "JPEG Save 100x100 RGB888",
		 .tst_fn = test_save_jpg,
		 .data = (void*)GP_PIXEL_RGB888,
		 .flags = TST_CHECK_MALLOC},

		{.name = "JPEG Save 100x100 BGR888",
		 .tst_fn = test_save_jpg,
		 .data = (void*)GP_PIXEL_BGR888,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
