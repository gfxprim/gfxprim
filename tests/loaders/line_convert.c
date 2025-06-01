// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <core/gp_convert.h>
#include <core/gp_get_put_pixel.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

struct test_data {
	gp_pixel r, g, b;
};

struct test_data colors[] = {
	{0x00, 0x00, 0x00},
	{0xff, 0x00, 0x00},
	{0x00, 0xff, 0x00},
	{0x00, 0x00, 0xff}
};

#define MUL 40
#define THRESHOLD 5

#define SYM_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))

static int xRGB_to_BGR_jpg(void)
{
	gp_pixmap *src, *res;
	unsigned int x;
	int ret = TST_PASSED;

	src = gp_pixmap_alloc(GP_ARRAY_SIZE(colors)*MUL, 1, GP_PIXEL_xRGB8888);

	if (!src) {
		tst_msg("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	for (x = 0; x < src->w; x++) {
		gp_pixel p = gp_rgb_to_pixel(colors[x/MUL].r, colors[x/MUL].g,
		                             colors[x/MUL].b, GP_PIXEL_xRGB8888);
		gp_putpixel(src, x, 0, p);
	}

	if (gp_save_jpg(src, "test.jpg", NULL)) {
		if (errno == ENOSYS) {
			tst_msg("jpeg not supported");
			return TST_UNTESTED;
		}

		tst_msg("Saving failed");
		return TST_FAILED;
	}

	gp_pixmap_free(src);

	res = gp_load_jpg("test.jpg", NULL);
	if (!res) {
		tst_msg("Failed to load");
		return TST_FAILED;
	}

	if (res->pixel_type != GP_PIXEL_BGR888) {
		tst_msg("Loaded jpg with wrong pixel type %s",
			gp_pixel_type_name(res->pixel_type));
		return TST_FAILED;
	}

	for (x = MUL/2; x < res->w; x+=MUL) {
		int fail = 0;

		gp_pixel p = gp_getpixel(res, x, 0);

		gp_pixel out_r = GP_PIXEL_GET_R_BGR888(p);
		gp_pixel out_g = GP_PIXEL_GET_G_BGR888(p);
		gp_pixel out_b = GP_PIXEL_GET_B_BGR888(p);

		gp_pixel in_r = colors[x/MUL].r;
		gp_pixel in_g = colors[x/MUL].g;
		gp_pixel in_b = colors[x/MUL].b;

		if (SYM_DIFF(in_r, out_r) > THRESHOLD) {
			tst_msg("Pixel red channel is wrong %02x expected %02x", out_r, in_r);
			fail = 1;
		}

		if (SYM_DIFF(in_g, out_g) > THRESHOLD) {
			tst_msg("Pixel green channel is wrong %02x expected %02x", out_g, in_g);
			fail = 1;
		}

		if (SYM_DIFF(in_b, out_b) > THRESHOLD) {
			tst_msg("Pixel blue channel is wrong %02x expected %02x", out_b, in_b);
			fail = 1;
		}

		if (fail) {
			tst_msg("Wrong pixel at %i %06x expected %02x%02x%02x", x, p, in_b, in_g, in_r);
			ret = TST_FAILED;
		}
	}

	gp_pixmap_free(res);

	return ret;
}

enum save_func {
	SAVE_PNM,
	SAVE_PGM,
	SAVE_PBM,
	SAVE_PNG,
	SAVE_TIF,
	SAVE_BMP,
};

struct test {
	enum save_func func;
	gp_pixel save_ptype;
	gp_pixel load_ptype;
	uint8_t is_rgb;
	uint8_t mod;
};

static int test_line_convert(struct test *test)
{
	gp_pixmap *save, *load;
	gp_pixel x, y;
	int ret;

	save = gp_pixmap_alloc(10, 10, test->save_ptype);

	for (x = 0; x < 10; x++) {
		for (y = 0; y < 10; y++) {
			gp_pixel val;

			if (test->is_rgb)
				val = gp_rgb_to_pixel(x, y, 0, save->pixel_type);
			else
				val = (x + y) % test->mod;

			gp_putpixel(save, x, y, val);
		}
	}

	switch (test->func) {
	case SAVE_PBM:
		ret = gp_save_pbm(save, "test.pbm", NULL);
	break;
	case SAVE_PGM:
		ret = gp_save_pgm(save, "test.pgm", NULL);
	break;
	case SAVE_PNM:
		ret = gp_save_pgm(save, "test.pnm", NULL);
	break;
	case SAVE_PNG:
		ret = gp_save_png(save, "test.png", NULL);
	break;
	case SAVE_TIF:
		ret = gp_save_tiff(save, "test.tif", NULL);
	break;
	case SAVE_BMP:
		ret = gp_save_bmp(save, "test.bmp", NULL);
	break;
	default:
		tst_msg("Invalid save func");
		return TST_FAILED;
	}

	if (ret) {
		if (errno == ENOSYS) {
			tst_msg("Not supported");
			return TST_UNTESTED;
		}

		tst_msg("Failed to save: %s", strerror(errno));
		return TST_FAILED;
	}

	gp_pixmap_free(save);

	switch (test->func) {
	case SAVE_PBM:
		load = gp_load_pbm("test.pbm", NULL);
	break;
	case SAVE_PGM:
		load = gp_load_pgm("test.pgm", NULL);
	break;
	case SAVE_PNM:
		load = gp_load_pnm("test.pnm", NULL);
	break;
	case SAVE_PNG:
		load = gp_load_png("test.png", NULL);
	break;
	case SAVE_TIF:
		load = gp_load_tiff("test.tif", NULL);
	break;
	case SAVE_BMP:
		load = gp_load_bmp("test.bmp", NULL);
	break;
	default:
		tst_msg("Invalid save func");
		return TST_FAILED;
	}

	if (!load) {
		tst_msg("Failed to load");
		return TST_FAILED;
	}

	if (load->pixel_type != test->load_ptype) {
		tst_msg("Loaded wrong pixel type %s expected %s",
			gp_pixel_type_name(load->pixel_type),
			gp_pixel_type_name(test->load_ptype));
		return TST_FAILED;
	}

	for (x = 0; x < 10; x++) {
		for (y = 0; y < 10; y++) {
			gp_pixel val;

			if (test->is_rgb)
				val = gp_rgb_to_pixel(x, y, 0, load->pixel_type);
			else
				val = (x + y) % test->mod;

			if (gp_getpixel(load, x, y) != val) {
				tst_msg("Wrong pixel value at %u %u 0x%08x != 0x%08x",
					(unsigned int)x, (unsigned int)y,
					gp_getpixel(load, x, y), val);
				return TST_FAILED;
			}
		}
	}

	gp_pixmap_free(load);
	return TST_PASSED;
}

static struct test test_1bpp_pbm = {
	.func = SAVE_PBM,
	.save_ptype = GP_PIXEL_G1_DB,
	.load_ptype = GP_PIXEL_G1,
	.mod = 2,
};

static struct test test_1bpp_pgm = {
	.func = SAVE_PGM,
	.save_ptype = GP_PIXEL_G1_DB,
	.load_ptype = GP_PIXEL_G1,
	.mod = 2,
};

static struct test test_2bpp_pgm = {
	.func = SAVE_PGM,
	.save_ptype = GP_PIXEL_G2_DB,
	.load_ptype = GP_PIXEL_G2,
	.mod = 4,
};

static struct test test_4bpp_pgm = {
	.func = SAVE_PGM,
	.save_ptype = GP_PIXEL_G4_DB,
	.load_ptype = GP_PIXEL_G4,
	.mod = 8,
};

static struct test test_1bpp_pnm = {
	.func = SAVE_PNM,
	.save_ptype = GP_PIXEL_G1_DB,
	.load_ptype = GP_PIXEL_G1,
	.mod = 2,
};

static struct test test_2bpp_pnm = {
	.func = SAVE_PNM,
	.save_ptype = GP_PIXEL_G2_DB,
	.load_ptype = GP_PIXEL_G2,
	.mod = 4,
};

static struct test test_4bpp_pnm = {
	.func = SAVE_PNM,
	.save_ptype = GP_PIXEL_G4_DB,
	.load_ptype = GP_PIXEL_G4,
	.mod = 8,
};

static struct test test_1bpp_png = {
	.func = SAVE_PNM,
	.save_ptype = GP_PIXEL_G1_DB,
	.load_ptype = GP_PIXEL_G1,
	.mod = 2,
};

static struct test test_2bpp_png = {
	.func = SAVE_PNM,
	.save_ptype = GP_PIXEL_G2_DB,
	.load_ptype = GP_PIXEL_G2,
	.mod = 4,
};

static struct test test_4bpp_png = {
	.func = SAVE_PNM,
	.save_ptype = GP_PIXEL_G4_DB,
	.load_ptype = GP_PIXEL_G4,
	.mod = 8,
};

static struct test test_rgb_png = {
	.func = SAVE_PNG,
	.save_ptype = GP_PIXEL_RGB888,
	.load_ptype = GP_PIXEL_BGR888,
	.is_rgb = 1,
};

static struct test test_xrgb_png = {
	.func = SAVE_PNG,
	.save_ptype = GP_PIXEL_xRGB8888,
	.load_ptype = GP_PIXEL_BGR888,
	.is_rgb = 1,
};

static struct test test_1bpp_tiff = {
	.func = SAVE_TIF,
	.save_ptype = GP_PIXEL_G1_DB,
	.load_ptype = GP_PIXEL_G1,
	.mod = 2,
};

static struct test test_2bpp_tiff = {
	.func = SAVE_TIF,
	.save_ptype = GP_PIXEL_G2_DB,
	.load_ptype = GP_PIXEL_G2,
	.mod = 4,
};

static struct test test_4bpp_tiff = {
	.func = SAVE_TIF,
	.save_ptype = GP_PIXEL_G4_DB,
	.load_ptype = GP_PIXEL_G4,
	.mod = 8,
};

static struct test test_rgb_tiff = {
	.func = SAVE_TIF,
	.save_ptype = GP_PIXEL_RGB888,
	.load_ptype = GP_PIXEL_BGR888,
	.is_rgb = 1,
};

static struct test test_xrgb_tiff = {
	.func = SAVE_TIF,
	.save_ptype = GP_PIXEL_xRGB8888,
	.load_ptype = GP_PIXEL_BGR888,
	.is_rgb = 1,
};

static struct test test_rgb_bmp = {
	.func = SAVE_BMP,
	.save_ptype = GP_PIXEL_BGR888,
	.load_ptype = GP_PIXEL_RGB888,
	.is_rgb = 1,
};

static struct test test_xrgb_bmp = {
	.func = SAVE_BMP,
	.save_ptype = GP_PIXEL_xRGB8888,
	.load_ptype = GP_PIXEL_RGB888,
	.is_rgb = 1,
};

const struct tst_suite tst_suite = {
	.suite_name = "Line convert",
	.tests = {
		{.name = "JPEG Line convert xRGB to BGR",
		 .tst_fn = xRGB_to_BGR_jpg,
		 .flags = TST_TMPDIR},

		{.name = "PBM Line convert 1bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_1bpp_pbm,
		 .flags = TST_TMPDIR},

		{.name = "PGM Line convert 1bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_1bpp_pgm,
		 .flags = TST_TMPDIR},

		{.name = "PGM Line convert 2bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_2bpp_pgm,
		 .flags = TST_TMPDIR},

		{.name = "PGM Line convert 4bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_4bpp_pgm,
		 .flags = TST_TMPDIR},

		{.name = "PNM Line convert 1bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_1bpp_pnm,
		 .flags = TST_TMPDIR},

		{.name = "PNM Line convert 2bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_2bpp_pnm,
		 .flags = TST_TMPDIR},

		{.name = "PNM Line convert 4bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_4bpp_pnm,
		 .flags = TST_TMPDIR},

		{.name = "PNG Line convert 1bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_1bpp_png,
		 .flags = TST_TMPDIR},

		{.name = "PNG Line convert 2bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_2bpp_png,
		 .flags = TST_TMPDIR},

		{.name = "PNG Line convert 4bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_4bpp_png,
		 .flags = TST_TMPDIR},

		{.name = "PNG Line convert RGB888",
		 .tst_fn = test_line_convert,
		 .data = &test_rgb_png,
		 .flags = TST_TMPDIR},

		{.name = "PNG Line convert xRGB8888",
		 .tst_fn = test_line_convert,
		 .data = &test_xrgb_png,
		 .flags = TST_TMPDIR},

		{.name = "TIFF Line convert 1bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_1bpp_tiff,
		 .flags = TST_TMPDIR},

		{.name = "TIFF Line convert 2bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_2bpp_tiff,
		 .flags = TST_TMPDIR},

		{.name = "TIFF Line convert 4bpp",
		 .tst_fn = test_line_convert,
		 .data = &test_4bpp_tiff,
		 .flags = TST_TMPDIR},

		{.name = "TIFF Line convert RGB888",
		 .tst_fn = test_line_convert,
		 .data = &test_rgb_tiff,
		 .flags = TST_TMPDIR},

		{.name = "TIFF Line convert xRGB8888",
		 .tst_fn = test_line_convert,
		 .data = &test_xrgb_tiff,
		 .flags = TST_TMPDIR},

		{.name = "BMP Line convert RGB888",
		 .tst_fn = test_line_convert,
		 .data = &test_rgb_bmp,
		 .flags = TST_TMPDIR},

		{.name = "BMP Line convert xRGB8888",
		 .tst_fn = test_line_convert,
		 .data = &test_xrgb_bmp,
		 .flags = TST_TMPDIR},

		{.name = NULL},
	}
};
