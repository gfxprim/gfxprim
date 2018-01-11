@ include source.t
/*
 * Tests that lines horizontally and vertically symetric lines are symetric.
 *
 * Copyright (C) 2018 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>

#include <core/GP_Pixmap.h>
#include <gfx/GP_Gfx.h>
#include <filters/GP_Rotate.h>

#include "tst_test.h"

@ max_x = 21
@ max_y = 21

static void print_diff(gp_pixmap *p1, gp_pixmap *p2)
{
	gp_size x, y;

	printf(" ");
	for (x = 0; x < p1->w; x++)
		printf("-");
	printf("\n");

	for (y = 0; y < p1->h; y++) {
		printf("|");
		for (x = 0; x < p1->w; x++) {
			gp_pixel px1 = gp_getpixel_raw_8BPP(p1, x, y);
			gp_pixel px2 = gp_getpixel_raw_8BPP(p2, x, y);

			if (px1 == px2) {
				if (px1)
					printf("*");
				else
					printf(" ");
			} else {
				if (px1)
					printf("1");
				else
					printf("2");
			}
		}
		printf("|\n");
	}

	printf(" ");
	for (x = 0; x < p1->w; x++)
		printf("-");
	printf("\n");
}

static void print(gp_pixmap *p)
{
	gp_size x, y;

	printf(" ");
	for (x = 0; x < p->w; x++)
		printf("-");
	printf("\n");

	for (y = 0; y < p->h; y++) {
		printf("|");
		for (x = 0; x < p->w; x++) {
			gp_pixel px = gp_getpixel_raw_8BPP(p, x, y);

			if (px)
				printf("*");
			else
				printf(" ");
		}
		printf("|\n");
	}

	printf(" ");
	for (x = 0; x < p->w; x++)
		printf("-");
	printf("\n");
}

static int compare_pixmaps(gp_pixmap *p1, gp_pixmap *p2)
{
	gp_size x, y;

	for (x = 0; x < p1->w; x++) {
		for (y = 0; y < p1->h; y++) {
			gp_pixel px1 = gp_getpixel_raw_8BPP(p1, x, y);
			gp_pixel px2 = gp_getpixel_raw_8BPP(p2, x, y);

			if (px1 != px2) {
				print(p1);
				print(p2);
				print_diff(p1, p2);
				return TST_FAILED;
			}
		}
	}

	return TST_SUCCESS;
}

static void prep(gp_pixmap **p1, gp_pixmap **p2, gp_size w, gp_size h)
{
	*p1 = gp_pixmap_alloc(w, h, GP_PIXEL_G8);
	*p2 = gp_pixmap_alloc(w, h, GP_PIXEL_G8);

	gp_fill(*p1, 0);
	gp_fill(*p2, 0);
}

@ for x in range(1, max_x):
@     for y in range(1, max_y):
static int line_{{ x }}_{{ y }}_h(void)
{
	gp_pixmap *p1, *p2;

	prep(&p1, &p2, {{ x }}+1, {{ y }}+1);

	gp_line(p1, 0, 0, {{ x }}, {{ y }}, 1);
	gp_line(p2, 0, {{ y }}, {{ x }}, 0, 1);

	gp_filter_mirror_h(p2, p2, NULL);

	return compare_pixmaps(p1, p2);
}

static int line_{{ x }}_{{ y }}_v(void)
{
	gp_pixmap *p1, *p2;

	prep(&p1, &p2, {{ x }}+1, {{ y }}+1);

	gp_line(p1, 0, 0, {{ x }}, {{ y }}, 1);
	gp_line(p2, {{ x }}, 0, 0, {{ y }}, 1);

	gp_filter_mirror_v(p2, p2, NULL);

	return compare_pixmaps(p1, p2);
}

static int line_{{ x }}_{{ y }}_rot(void)
{
	gp_pixmap *p1, *p2;
	gp_size w = GP_MAX({{ x }}, {{ y }}) + 1;

	prep(&p1, &p2, w, w);

	gp_line(p1, 0, 0, {{ x }}, {{ y }}, 1);
	gp_line(p2, 0, w-1, {{ y }}, w - 1 - {{ x }}, 1);
	p2 = gp_filter_rotate_90_alloc(p2, NULL);

	return compare_pixmaps(p1, p2);
}

@ end
@
const struct tst_suite tst_suite = {
	.suite_name = "Line symmetry testsuite",
	.tests = {
@ for x in range(1, max_x):
@     for y in range(1, max_y):
		{.name = "line_h 0, 0, {{ x }}, {{ y }} vs 0, {{ y }}, {{ x }}, 0",
		 .tst_fn = line_{{ x }}_{{ y }}_h},
		{.name = "line_v 0, 0, {{ x }}, {{ y }} vs {{ x }}, 0, 0, {{ y }}",
		 .tst_fn = line_{{ x }}_{{ y }}_v},
		{.name = "line 0, 0, {{ x }}, {{ y }} rot 90",
		 .tst_fn = line_{{ x }}_{{ y }}_rot},
@ end
		{.name = NULL}
	}
};
