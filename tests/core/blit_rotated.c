// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Blit tests with rotated pixmaps.

  Compares gp_blit() against a reference that blits pixel by pixel with the
  rotation aware gp_getpixel() and gp_putpixel(). The source and destination
  have different sizes and the blitted rectangle is at an odd offset, so that
  wrongly transformed coordinates end up in a different place.

 */
#include <stdlib.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_convert.h>
#include <core/gp_blit.h>

#include "tst_test.h"

struct rotated_test {
	gp_pixel_type src_type;
	gp_pixel_type dst_type;
	/* Rotation flags, bit 0 axes_swap, bit 1 x_swap, bit 2 y_swap */
	int src_rot;
	int dst_rot;
};

static void fill_random(gp_pixmap *pixmap)
{
	uint32_t size = gp_pixel_size(pixmap->pixel_type);
	gp_pixel mask = size >= 32 ? 0xffffffff : (1u << size) - 1;
	gp_size x, y;

	for (y = 0; y < pixmap->h; y++) {
		for (x = 0; x < pixmap->w; x++)
			gp_putpixel_raw(pixmap, x, y, (gp_pixel)random() & mask);
	}
}

static void copy_raw(const gp_pixmap *src, gp_pixmap *dst)
{
	gp_size x, y;

	for (y = 0; y < src->h; y++) {
		for (x = 0; x < src->w; x++)
			gp_putpixel_raw(dst, x, y, gp_getpixel_raw(src, x, y));
	}
}

static void set_rotation(gp_pixmap *pixmap, int rot)
{
	gp_pixmap_rotation_set(pixmap, rot & 1, !!(rot & 2), !!(rot & 4));
}

static void ref_blit(const gp_pixmap *src, gp_coord x0, gp_coord y0,
                     gp_size w, gp_size h, gp_pixmap *dst, gp_coord x1, gp_coord y1)
{
	gp_size x, y;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			gp_pixel p = gp_getpixel(src, x0 + x, y0 + y);

			/* Conversion would clear the padding bits */
			if (src->pixel_type != dst->pixel_type)
				p = gp_convert_pixmap_pixel(p, src, dst);

			gp_putpixel(dst, x1 + x, y1 + y, p);
		}
	}
}

static int compare_raw(const gp_pixmap *res, const gp_pixmap *ref)
{
	gp_size x, y;

	for (y = 0; y < res->h; y++) {
		for (x = 0; x < res->w; x++) {
			gp_pixel p1 = gp_getpixel_raw(res, x, y);
			gp_pixel p2 = gp_getpixel_raw(ref, x, y);

			if (p1 != p2) {
				tst_msg("Pixels differ at raw %u,%u: 0x%08x expected 0x%08x",
				        x, y, p1, p2);
				return 1;
			}
		}
	}

	return 0;
}

static int rotated_test(struct rotated_test *test)
{
	gp_pixmap *src, *res, *ref;
	int ret = TST_PASSED;

	src = gp_pixmap_alloc(37, 23, test->src_type);
	res = gp_pixmap_alloc(51, 45, test->dst_type);
	ref = gp_pixmap_alloc(51, 45, test->dst_type);

	if (!src || !res || !ref) {
		tst_msg("gp_pixmap_alloc() failed");
		ret = TST_UNTESTED;
		goto exit;
	}

	fill_random(src);
	fill_random(res);
	copy_raw(res, ref);

	set_rotation(src, test->src_rot);
	set_rotation(res, test->dst_rot);
	set_rotation(ref, test->dst_rot);

	gp_blit(src, 3, 2, 9, 11, res, 5, 7);
	ref_blit(src, 3, 2, 9, 11, ref, 5, 7);

	if (compare_raw(res, ref))
		ret = TST_FAILED;

exit:
	gp_pixmap_free(src);
	gp_pixmap_free(res);
	gp_pixmap_free(ref);

	return ret;
}

#define ROT_TEST(name, src, dst, srot, drot) \
	static struct rotated_test name = { \
		.src_type = src, \
		.dst_type = dst, \
		.src_rot = srot, \
		.dst_rot = drot, \
	}

#define ROT_TESTS(prefix, src, dst) \
	ROT_TEST(prefix##_0, src, dst, 0, 0); \
	ROT_TEST(prefix##_1, src, dst, 1, 1); \
	ROT_TEST(prefix##_2, src, dst, 2, 2); \
	ROT_TEST(prefix##_3, src, dst, 3, 3); \
	ROT_TEST(prefix##_4, src, dst, 4, 4); \
	ROT_TEST(prefix##_5, src, dst, 5, 5); \
	ROT_TEST(prefix##_6, src, dst, 6, 6); \
	ROT_TEST(prefix##_7, src, dst, 7, 7); \
	ROT_TEST(prefix##_diff1, src, dst, 0, 1); \
	ROT_TEST(prefix##_diff2, src, dst, 6, 3)

ROT_TESTS(xrgb8888, GP_PIXEL_xRGB8888, GP_PIXEL_xRGB8888);
ROT_TESTS(rgb888, GP_PIXEL_RGB888, GP_PIXEL_RGB888);
ROT_TESTS(rgb565, GP_PIXEL_RGB565, GP_PIXEL_RGB565);
ROT_TESTS(g1, GP_PIXEL_G1_DB, GP_PIXEL_G1_DB);
ROT_TESTS(rgb888_xrgb8888, GP_PIXEL_RGB888, GP_PIXEL_xRGB8888);
ROT_TESTS(xrgb8888_rgb565, GP_PIXEL_xRGB8888, GP_PIXEL_RGB565);

#define TEST(desc, tdata) \
	{.name = desc, .tst_fn = rotated_test, .data = &tdata}

#define TESTS(desc, prefix) \
	TEST(desc " no rotation", prefix##_0), \
	TEST(desc " both axes_swap", prefix##_1), \
	TEST(desc " both x_swap", prefix##_2), \
	TEST(desc " both axes_swap x_swap", prefix##_3), \
	TEST(desc " both y_swap", prefix##_4), \
	TEST(desc " both axes_swap y_swap", prefix##_5), \
	TEST(desc " both x_swap y_swap", prefix##_6), \
	TEST(desc " both axes_swap x_swap y_swap", prefix##_7), \
	TEST(desc " different rotation 1", prefix##_diff1), \
	TEST(desc " different rotation 2", prefix##_diff2)

const struct tst_suite tst_suite = {
	.suite_name = "Blit Rotated Testsuite",
	.tests = {
		TESTS("xRGB8888 -> xRGB8888", xrgb8888),
		TESTS("RGB888 -> RGB888", rgb888),
		TESTS("RGB565 -> RGB565", rgb565),
		TESTS("G1_DB -> G1_DB", g1),
		TESTS("RGB888 -> xRGB8888", rgb888_xrgb8888),
		TESTS("xRGB8888 -> RGB565", xrgb8888_rgb565),
		{.name = NULL},
	}
};
