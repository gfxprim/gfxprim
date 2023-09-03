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

struct tcase {
	gp_pixel_type pixel_type;
	gp_pixel pixel;
	gp_size pw;
	gp_size ph;

	gp_coord x;
	gp_coord y;
	gp_size w;
	gp_size h;

	unsigned int pixel_cnt;
	gp_coord *pixels;

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

	.pixel_cnt = 4,
	.pixels = (gp_coord[]) {
		0, 0,
		3, 1,
		/* out of subpixmap */
		10, 0,
		11, 1,
	},

	.exp_buf = {
		0x00, 0x00,
		0x40, 0x00,
		0x08, 0x00,
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

	.pixel_cnt = 4,
	.pixels = (gp_coord[]) {
		0, 0,
		3, 1,
		/* out of subpixmap */
		10, 0,
		11, 1,
	},

	.exp_buf = {
		0x00, 0x00,
		0x20, 0x00,
		0x04, 0x00,
		0x00, 0x00,
	}
};

static struct tcase sub_pixmap_1bpp_off3 = {
	.pixel_type = GP_PIXEL_G1,
	.pixel = 0x01,
	.pw = 16,
	.ph = 4,
	.x = 3,
	.y = 1,
	.w = 10,
	.h = 2,

	.pixel_cnt = 4,
	.pixels = (gp_coord[]) {
		0, 0,
		3, 1,
		/* out of subpixmap */
		10, 0,
		11, 1,
	},

	.exp_buf = {
		0x00, 0x00,
		0x10, 0x00,
		0x02, 0x00,
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

	.pixel_cnt = 4,
	.pixels = (gp_coord[]) {
		0, 0,
		3, 1,
		/* out of subpixmap */
		10, 0,
		11, 1,
	},

	.exp_buf = {
		0x00, 0x00,
		0x08, 0x00,
		0x01, 0x00,
		0x00, 0x00,
	}
};

static struct tcase sub_pixmap_1bpp_off0 = {
	.pixel_type = GP_PIXEL_G1,
	.pixel = 0x01,
	.pw = 16,
	.ph = 4,
	.x = 8,
	.y = 1,
	.w = 8,
	.h = 2,

	.pixel_cnt = 4,
	.pixels = (gp_coord[]) {
		0, 0,
		3, 1,
		/* out of subpixmap */
		10, 0,
		11, 1,
	},

	.exp_buf = {
		0x00, 0x00,
		0x00, 0x80,
		0x00, 0x10,
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

	.pixel_cnt = 4,
	.pixels = (gp_coord[]) {
		0, 0,
		3, 1,
		/* out of subpixmap */
		10, 0,
		11, 1,
	},

	.exp_buf = {
		0x00, 0x00,
		0x30, 0x00,
		0x00, 0xc0,
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

	.pixel_cnt = 4,
	.pixels = (gp_coord[]) {
		0, 0,
		3, 1,
		/* out of subpixmap */
		10, 0,
		11, 1,
	},

	.exp_buf = {
		0x00, 0x00,
		0x0c, 0x00,
		0x00, 0x30,
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

	.pixel_cnt = 4,
	.pixels = (gp_coord[]) {
		0, 0,
		3, 1,
		/* out of subpixmap */
		10, 0,
		11, 1,
	},

	.exp_buf = {
		0x00, 0x00,
		0x03, 0x00,
		0x00, 0x0c,
		0x00, 0x00,
	}
};

static struct tcase sub_pixmap_4bpp_off1 = {
	.pixel_type = GP_PIXEL_G4,
	.pixel = 0x0f,
	.pw = 4,
	.ph = 4,
	.x = 1,
	.y = 1,
	.w = 2,
	.h = 2,

	.pixel_cnt = 4,
	.pixels = (gp_coord[]) {
		0, 0,
		1, 1,
		/* out of subpixmap */
		10, 0,
		11, 1,
	},

	.exp_buf = {
		0x00, 0x00,
		0x0f, 0x00,
		0x00, 0xf0,
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

static int sub_pixmap_putpixel(struct tcase *tc)
{
	gp_pixmap *p, s;
	unsigned int i;

	p = gp_pixmap_alloc(tc->pw, tc->ph, tc->pixel_type);
	if (!p) {
		tst_msg("malloc failed!");
		return TST_FAILED;
	}

	gp_fill(p, 0x00);

	gp_sub_pixmap(p, &s, tc->x, tc->y, tc->w, tc->h);

	for (i = 0; i < tc->pixel_cnt; i++)
		gp_putpixel(&s, tc->pixels[2*i], tc->pixels[2*i+1], tc->pixel);

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
	.suite_name = "Subpixmap put pixel testsuite",
	.tests = {
		{.name = "Sub pixmap 1bpp off=1",
		 .tst_fn = sub_pixmap_putpixel,
		 .data = &sub_pixmap_1bpp_off1},
		{.name = "Sub pixmap 1bpp off=2",
		 .tst_fn = sub_pixmap_putpixel,
		 .data = &sub_pixmap_1bpp_off2},
		{.name = "Sub pixmap 1bpp off=3",
		 .tst_fn = sub_pixmap_putpixel,
		 .data = &sub_pixmap_1bpp_off3},
		{.name = "Sub pixmap 1bpp off=4",
		 .tst_fn = sub_pixmap_putpixel,
		 .data = &sub_pixmap_1bpp_off4},
		{.name = "Sub pixmap 1bpp off=0",
		 .tst_fn = sub_pixmap_putpixel,
		 .data = &sub_pixmap_1bpp_off0},
		{.name = "Sub pixmap 2bpp off=1",
		 .tst_fn = sub_pixmap_putpixel,
		 .data = &sub_pixmap_2bpp_off1},
		{.name = "Sub pixmap 2bpp off=2",
		 .tst_fn = sub_pixmap_putpixel,
		 .data = &sub_pixmap_2bpp_off2},
		{.name = "Sub pixmap 2bpp off=3",
		 .tst_fn = sub_pixmap_putpixel,
		 .data = &sub_pixmap_2bpp_off3},
		{.name = "Sub pixmap 4bpp off=1",
		 .tst_fn = sub_pixmap_putpixel,
		 .data = &sub_pixmap_4bpp_off1},
		{.name = NULL},
	}
};
