@ include source.t
/*
 * Copyright (C) 2018 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>

#include <core/GP_Pixmap.h>
#include <gfx/GP_Gfx.h>
#include <filters/GP_Rotate.h>

#include "tst_test.h"
#include "compare.h"

@ max_x = 10
@ max_y = 3

static void prep(gp_pixmap **p1, gp_pixmap **p2, gp_size w, gp_size h)
{
	*p1 = gp_pixmap_alloc(w, h, GP_PIXEL_G8);
	*p2 = gp_pixmap_alloc(w, h, GP_PIXEL_G8);

	gp_fill(*p1, 0);
	gp_fill(*p2, 0);
}

@ for x in range(2, max_x, 2):
@     for y in range(1, max_y):
static int isosceles_symmetry_{{ x }}_{{ y }}(void)
{
	gp_pixmap *p1, *p2;

	prep(&p1, &p2, {{ x }}+1, {{ y }}+1);

	gp_fill_triangle(p1, {{ x }}/2, 0, 0, {{ y }}, {{ x }}, {{ y }}, 1);
	gp_fill_triangle(p2, {{ x }}/2, 0, 0, {{ y }}, {{ x }}, {{ y }}, 1);

	gp_filter_mirror_h(p2, p2, NULL);

	return compare_pixmaps(p1, p2);
}

@ end
@
const struct tst_suite tst_suite = {
	.suite_name = "Triangle fill testsuite",
	.tests = {
@ for x in range(2, max_x, 2):
@     for y in range(1, max_y):
		{.name = "Isosceles symmetry {{ x }}, {{ y }}",
		 .tst_fn = isosceles_symmetry_{{ x }}_{{ y }}},
@ end
		{.name = NULL}
	}
};
