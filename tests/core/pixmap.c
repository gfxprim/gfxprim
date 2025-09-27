// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <string.h>
#include <core/gp_pixmap.h>
#include <core/gp_fill.h>
#include <core/gp_get_put_pixel.h>
#include "tst_test.h"

static int pixmap_alloc_free(void)
{
	gp_pixmap *p;

	p = gp_pixmap_alloc(100, 200, GP_PIXEL_RGB888);

	if (!p) {
		tst_msg("gp_pixmap_alloc() failed");
		return TST_FAILED;
	}

	TST_EXP_UINT(p->w, 100, "Width should be:");
	TST_EXP_UINT(p->h, 200, "Height should be:");
	TST_EXP_UINT(p->bytes_per_row, 3 * p->w, "Bytes per row should be:");
	TST_EXP_UINT(p->offset, 0, "Offset should be:");
	TST_EXP_NULL(p->gamma, "Pixmap gamma");

	if (p->pixel_type != GP_PIXEL_RGB888) {
		tst_msg("pixmap->pixel_type != GP_PIXEL_RGB888");
		return TST_FAILED;
	}

	if (p->axes_swap != 0 || p->x_swap != 0 || p->y_swap != 0) {
		tst_msg("Wrong default orientation %i %i %i",
		        p->axes_swap, p->x_swap, p->y_swap);
		return TST_FAILED;
	}

	/* access the pixel buffer */
	*(char*)GP_PIXEL_ADDR(p, 0, 0) = 0;
	*(char*)GP_PIXEL_ADDR(p, 100, 100) = 0;

	gp_pixmap_free(p);

	return TST_PASSED;
}

static int subpixmap_assert(const gp_pixmap *p, const gp_pixmap *sp,
                            gp_size w, gp_size h)
{
	TST_EXP_UINT(sp->w, w, "sub_pixmap->w: ");
	TST_EXP_UINT(sp->h, h, "sub_pixmap->h:");
	TST_EXP_UINT(sp->bytes_per_row, p->bytes_per_row, "pixmap->bytes_per_row != sub_pixmap->bytes_per_row");
	TST_EXP_UINT(sp->offset, 0, "Offset should be:");
	TST_EXP_NULL(sp->gamma, "sub_pixmap->gamma");

	if (sp->pixel_type != GP_PIXEL_RGB888) {
		tst_msg("sub_pixmap->pixel_type != GP_PIXEL_RGB888");
		return TST_FAILED;
	}

	if (sp->axes_swap != 0 || sp->x_swap != 0 || sp->y_swap != 0) {
		tst_msg("Wrong default orientation %i %i %i",
		        sp->axes_swap, sp->x_swap, sp->y_swap);
		return TST_FAILED;
	}

	/* access the start and end of the pixel buffer */
	*(char*)GP_PIXEL_ADDR(sp, 0, 0) = 0;
	*(char*)GP_PIXEL_ADDR(sp, sp->w - 1, sp->h - 1) = 0;

	return 0;
}

static int sub_pixmap_alloc_free(void)
{
	gp_pixmap *c, *sc;
	int ret;

	c = gp_pixmap_alloc(300, 300, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_msg("gp_pixmap_alloc() failed");
		return TST_UNTESTED;
	}

	sc = gp_sub_pixmap_alloc(c, 100, 100, 100, 100);

	if (!sc) {
		gp_pixmap_free(c);
		return TST_FAILED;
	}

	ret = subpixmap_assert(c, sc, 100, 100);

	if (ret)
		return ret;

	gp_pixmap_free(c);
	gp_pixmap_free(sc);

	return TST_PASSED;
}

static int sub_pixmap_create(void)
{
	gp_pixmap *c, sc;
	int ret;

	c = gp_pixmap_alloc(300, 300, GP_PIXEL_RGB888);

	if (c == NULL) {
		tst_msg("gp_pixmap_alloc() failed");
		return TST_UNTESTED;
	}

	gp_sub_pixmap(c, &sc, 100, 100, 100, 100);

	ret = subpixmap_assert(c, &sc, 100, 100);

	if (ret)
		return ret;

	gp_pixmap_free(c);

	return TST_PASSED;
}

static int pixmap_zero_w(void)
{
	gp_pixmap *p;

	p = gp_pixmap_alloc(0, 200, GP_PIXEL_G8);
	if (!p) {
		tst_msg("Pixmap with zero width wasn't allocated");
		return TST_FAILED;
	}

	TST_EXP_NULL(p->pixels, "Pixmap pixels");
	TST_EXP_UINT(p->w, 0, "Width should be:");
	TST_EXP_UINT(p->h, 200, "Height should be:");
	TST_EXP_UINT(p->bytes_per_row, 0, "Bytes per row should be:");
	TST_EXP_UINT(p->offset, 0, "Offset should be:");
	TST_EXP_NULL(p->gamma, "Pixmap gamma");

	if (p->axes_swap != 0 || p->x_swap != 0 || p->y_swap != 0) {
		tst_msg("Wrong default orientation %i %i %i",
		        p->axes_swap, p->x_swap, p->y_swap);
		return TST_FAILED;
	}

	gp_pixmap_free(p);

	return TST_PASSED;
}

static int pixmap_zero_h(void)
{
	gp_pixmap *p;

	p = gp_pixmap_alloc(200, 0, GP_PIXEL_G8);
	if (!p) {
		tst_msg("Pixmap with zero height wasn't allocated");
		return TST_FAILED;
	}

	TST_EXP_NULL(p->pixels, "Pixmap pixels");
	TST_EXP_UINT(p->w, 200, "Width should be:");
	TST_EXP_UINT(p->h, 0, "Height should be:");
	TST_EXP_UINT(p->bytes_per_row, 0, "Bytes per row should be:");
	TST_EXP_UINT(p->offset, 0, "Offset should be:");
	TST_EXP_NULL(p->gamma, "Pixmap gamma");

	if (p->axes_swap != 0 || p->x_swap != 0 || p->y_swap != 0) {
		tst_msg("Wrong default orientation %i %i %i",
		        p->axes_swap, p->x_swap, p->y_swap);
		return TST_FAILED;
	}

	gp_pixmap_free(p);

	return TST_PASSED;
}

static int pixmap_invalid_pixeltype1(void)
{
	gp_pixmap *c;

	c = gp_pixmap_alloc(100, 100, -1);

	if (c) {
		tst_msg("Pixmap with invalid pixel type (-1) succesfully allocated");
		return TST_FAILED;
	}

	if (errno != EINVAL) {
		tst_msg("Expected errno set to EINVAL");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int pixmap_invalid_pixeltype2(void)
{
	gp_pixmap *c;

	c = gp_pixmap_alloc(100, 100, GP_PIXEL_MAX + 1000);

	if (c != NULL) {
		tst_msg("Pixmap with invalid pixel type (-1) succesfully allocated");
		return TST_FAILED;
	}

	if (errno != EINVAL) {
		tst_msg("Expected errno set to EINVAL");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int pixmap_from_data(void)
{
	gp_pixmap *p;
	unsigned char *data = malloc(100);
	unsigned int i, j;

	memset(data, 127, 100);

	p = gp_pixmap_from_data(10, 10, GP_PIXEL_G8, data, GP_PIXMAP_FREE_PIXELS);
	if (!p) {
		tst_msg("Malloc failed :(");
		return TST_FAILED;
	}

	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			if (gp_getpixel(p, i, j) != 127) {
				tst_msg("Wrong data in pixmap at %u, %u!", i, j);
				return TST_FAILED;
			}
		}
	}

	gp_pixmap_free(p);
	return TST_PASSED;
}

static int pixmap_from_static_data(void)
{
	gp_pixmap *p;
	unsigned char data[] = {0, 0, 0,
	                        0, 1, 0,
	                        0, 0, 0};

	p = gp_pixmap_from_data(3, 3, GP_PIXEL_G8, data, 0);

	if (gp_getpixel(p, 1, 1) != 1) {
		tst_msg("Wrong data at 1, 1");
		return TST_FAILED;
	}

	gp_pixmap_free(p);
	return TST_PASSED;
}

struct tcase {
	gp_pixel_type pixel_type;
	gp_pixel pixel;
	gp_size pw;
	gp_size ph;

	gp_coord x;
	gp_coord y;
	gp_size w;
	gp_size h;

	uint8_t exp_off;

	uint8_t exp_buf[];
};

static struct tcase sub_pixmap_1bpp_off1 = {
	.pixel_type = GP_PIXEL_G1,
	.pixel = 0x01,
	.pw = 16,
	.ph = 4,
	.x = 1,
	.y = 1,
	.w = 10,
	.h = 2,
	.exp_off = 1,
	.exp_buf = {
		0x00, 0x00,
		0x7f, 0xe0,
		0x7f, 0xe0,
		0x00, 0x00,
	}
};

static struct tcase sub_pixmap_1bpp_off2 = {
	.pixel_type = GP_PIXEL_G1,
	.pixel = 0x01,
	.pw = 16,
	.ph = 4,
	.x = 2,
	.y = 1,
	.w = 10,
	.h = 2,
	.exp_off = 2,
	.exp_buf = {
		0x00, 0x00,
		0x3f, 0xf0,
		0x3f, 0xf0,
		0x00, 0x00,
	}
};

static struct tcase sub_pixmap_1bpp_off4 = {
	.pixel_type = GP_PIXEL_G1,
	.pixel = 0x01,
	.pw = 16,
	.ph = 4,
	.x = 4,
	.y = 1,
	.w = 10,
	.h = 2,
	.exp_off = 4,
	.exp_buf = {
		0x00, 0x00,
		0x0f, 0xfc,
		0x0f, 0xfc,
		0x00, 0x00,
	}
};

static struct tcase sub_pixmap_2bpp_off1 = {
	.pixel_type = GP_PIXEL_G2,
	.pixel = 0x03,
	.pw = 8,
	.ph = 4,
	.x = 1,
	.y = 1,
	.w = 4,
	.h = 2,
	.exp_off = 1,
	.exp_buf = {
		0x00, 0x00,
		0x3f, 0xc0,
		0x3f, 0xc0,
		0x00, 0x00,
	}
};

static struct tcase sub_pixmap_2bpp_off2 = {
	.pixel_type = GP_PIXEL_G2,
	.pixel = 0x03,
	.pw = 8,
	.ph = 4,
	.x = 2,
	.y = 1,
	.w = 4,
	.h = 2,
	.exp_off = 2,
	.exp_buf = {
		0x00, 0x00,
		0x0f, 0xf0,
		0x0f, 0xf0,
		0x00, 0x00,
	}
};

static struct tcase sub_pixmap_2bpp_off3 = {
	.pixel_type = GP_PIXEL_G2,
	.pixel = 0x03,
	.pw = 8,
	.ph = 4,
	.x = 3,
	.y = 1,
	.w = 4,
	.h = 2,
	.exp_off = 3,
	.exp_buf = {
		0x00, 0x00,
		0x03, 0xfc,
		0x03, 0xfc,
		0x00, 0x00,
	}
};

static struct tcase sub_pixmap_2bpp_off0 = {
	.pixel_type = GP_PIXEL_G2,
	.pixel = 0x03,
	.pw = 8,
	.ph = 4,
	.x = 4,
	.y = 1,
	.w = 4,
	.h = 2,
	.exp_off = 0,
	.exp_buf = {
		0x00, 0x00,
		0x00, 0xff,
		0x00, 0xff,
		0x00, 0x00,
	}
};

static void dump_buf(const char *name, uint8_t *buf, size_t size)
{
	size_t i;

	printf("%s", name);

	for (i = 0; i < size; i++)
		printf("0x%02x ", buf[i]);

	printf("\n");
}

static int sub_pixmap_off(struct tcase *tc)
{
	gp_pixmap *p, s;

	p = gp_pixmap_alloc(tc->pw, tc->ph, tc->pixel_type);
	if (!p) {
		tst_msg("malloc failed!");
		return TST_FAILED;
	}

	gp_fill(p, 0x00);

	gp_sub_pixmap(p, &s, tc->x, tc->y, tc->w, tc->h);

	if (s.offset != tc->exp_off) {
		tst_msg("Wrong offset %u expected %u",
			(unsigned int)s.offset, (unsigned int)tc->exp_off);
		return TST_FAILED;
	}

	gp_fill(&s, tc->pixel);

	size_t size = p->bytes_per_row * p->h;

	if (memcmp(tc->exp_buf, p->pixels, size)) {
		tst_msg("Wrong bitmap data!");
		dump_buf("written:  ", p->pixels, size);
		dump_buf("expected: ", tc->exp_buf, size);
		return TST_FAILED;
	}

	gp_pixmap_free(p);
	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "Pixmap Testsuite",
	.tests = {
		{.name = "Pixmap alloc free",
		 .tst_fn = pixmap_alloc_free,
		 .flags = TST_CHECK_MALLOC},
		{.name = "Sub pixmap alloc free",
		 .tst_fn = sub_pixmap_alloc_free,
		 .flags = TST_CHECK_MALLOC},
		{.name = "Pixmap from data",
		 .tst_fn = pixmap_from_data,
		 .flags = TST_CHECK_MALLOC},
		{.name = "Pixmap from static data",
		 .tst_fn = pixmap_from_static_data,
		 .flags = TST_CHECK_MALLOC},
		{.name = "Sub pixmap create",
		 .tst_fn = sub_pixmap_create},
		{.name = "Sub pixmap 1bpp off=1",
		 .tst_fn = sub_pixmap_off,
		 .data = &sub_pixmap_1bpp_off1},
		{.name = "Sub pixmap 1bpp off=2",
		 .tst_fn = sub_pixmap_off,
		 .data = &sub_pixmap_1bpp_off2},
		{.name = "Sub pixmap 1bpp off=4",
		 .tst_fn = sub_pixmap_off,
		 .data = &sub_pixmap_1bpp_off4},
		{.name = "Sub pixmap 2bpp off=1",
		 .tst_fn = sub_pixmap_off,
		 .data = &sub_pixmap_2bpp_off1},
		{.name = "Sub pixmap 2bpp off=2",
		 .tst_fn = sub_pixmap_off,
		 .data = &sub_pixmap_2bpp_off2},
		{.name = "Sub pixmap 2bpp off=3",
		 .tst_fn = sub_pixmap_off,
		 .data = &sub_pixmap_2bpp_off3},
		{.name = "Sub pixmap 2bpp off=0",
		 .tst_fn = sub_pixmap_off,
		 .data = &sub_pixmap_2bpp_off0},
		{.name = "Pixmap create w = 0",
		 .tst_fn = pixmap_zero_w,
		 .flags = TST_CHECK_MALLOC},
		{.name = "Pixmap create h = 0",
		 .tst_fn = pixmap_zero_h,
		 .flags = TST_CHECK_MALLOC},
		{.name = "Pixmap create pixel_type = -1",
		 .tst_fn = pixmap_invalid_pixeltype1},
		{.name = "Pixmap create invalid pixel_type",
		 .tst_fn = pixmap_invalid_pixeltype2},
		{.name = NULL},
	}
};
