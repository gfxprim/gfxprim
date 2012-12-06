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
#include <sys/stat.h>

#include <core/GP_Context.h>
#include <gfx/GP_Polygon.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* polygon description */
	unsigned int edge_count;
	GP_Coord edges[20];
	
	/* expected result */
	GP_Size w, h;
	const char pixmap[];
};

static int test_polygon(struct testcase *t)
{
	GP_Context *c;
	int err;

	c = GP_ContextAlloc(t->w, t->h, GP_PIXEL_G8);

	if (c == NULL) {
		tst_err("Failed to allocate context");
		return TST_UNTESTED;
	}

	/* zero the pixels buffer */
	memset(c->pixels, 0, c->w * c->h);

	GP_FillPolygon(c, t->edge_count, t->edges, 1);

	err = compare_buffers(t->pixmap, c);

	if (err) {
		tst_msg("Patterns are different");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

struct testcase testcase_1_edge = {
	.edge_count = 1,
	.edges = {
		1, 1,
	},
	.w = 3,
	.h = 3,
	.pixmap = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0,
	}
};

struct testcase testcase_5_edges_1px = {
	.edge_count = 5,
	.edges = {
		1, 1,
		1, 1,
		1, 1,
		1, 1,
		1, 1,
	},
	.w = 3,
	.h = 3,
	.pixmap = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0,
	}
};

struct testcase testcase_line_vert_3px = {
	.edge_count = 2,
	.edges = {
		1, 1,
		1, 3,
	},
	.w = 5,
	.h = 5,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_line_horiz_3px = {
	.edge_count = 2,
	.edges = {
		3, 1,
		1, 0,
	},
	.w = 5,
	.h = 5,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_line_4px = {
	.edge_count = 2,
	.edges = {
		1, 1,
		2, 3,
	},
	.w = 5,
	.h = 5,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 1, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_line_3px = {
	.edge_count = 2,
	.edges = {
		1, 1,
		3, 3,
	},
	.w = 5,
	.h = 5,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 1, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_2x2_square = {
	.edge_count = 4,
	.edges = {
		1, 1,
		2, 1,
		2, 2,
		1, 2,
	},
	.w = 4,
	.h = 4,
	.pixmap = {
		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,
	}
};

struct testcase testcase_3x3_square = {
	.edge_count = 4,
	.edges = {
		1, 1,
		3, 1,
		3, 3,
		1, 3,
	},
	.w = 5,
	.h = 5,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_4x4_square = {
	.edge_count = 4,
	.edges = {
		1, 1,
		4, 1,
		4, 4,
		1, 4,
	},
	.w = 6,
	.h = 6,
	.pixmap = {
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 0,
		0, 0, 0, 0, 0, 0,
	}
};

struct testcase testcase_4px_triangle = {
	.edge_count = 3,
	.edges = {
		2, 1,
		1, 2,
		3, 2,
	},
	.w = 5,
	.h = 4,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_6px_triangle = {
	.edge_count = 3,
	.edges = {
		1, 1,
		1, 3,
		3, 3,
	},
	.w = 5,
	.h = 5,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 1, 1, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
	}
};

static int test_1_edge(void)
{
	return test_polygon(&testcase_1_edge);
}

static int test_5_edges_1px(void)
{
	return test_polygon(&testcase_5_edges_1px);
}

static int test_line_vert_3px(void)
{
	return test_polygon(&testcase_line_vert_3px);
}

static int test_line_horiz_3px(void)
{
	return test_polygon(&testcase_line_horiz_3px);
}

static int test_line_3px(void)
{
	return test_polygon(&testcase_line_3px);
}

static int test_line_4px(void)
{
	return test_polygon(&testcase_line_4px);
}

static int test_2x2_square(void)
{
	return test_polygon(&testcase_2x2_square);
}

static int test_3x3_square(void)
{
	return test_polygon(&testcase_3x3_square);
}

static int test_4x4_square(void)
{
	return test_polygon(&testcase_4x4_square);
}

static int test_4px_triangle(void)
{
	return test_polygon(&testcase_4px_triangle);
}

static int test_6px_triangle(void)
{
	return test_polygon(&testcase_6px_triangle);
}

const struct tst_suite tst_suite = {
	.suite_name = "Polygon Testsuite",
	.tests = {
		{.name = "1 Edge Polygon", .tst_fn = test_1_edge},
		{.name = "5 Edges 1px Polygon", .tst_fn = test_5_edges_1px},
		{.name = "Vertical Line 3px Polygon", .tst_fn = test_line_vert_3px},
		{.name = "Horizonval Line 3px Polygon", .tst_fn = test_line_horiz_3px},
		{.name = "Line 3px Polygon", .tst_fn = test_line_3px},
		{.name = "Line 4px Polygon", .tst_fn = test_line_4px},
		{.name = "2x2 Square Polygon", .tst_fn = test_2x2_square},
		{.name = "3x3 Square Polygon", .tst_fn = test_3x3_square},
		{.name = "4x4 Square Polygon", .tst_fn = test_4x4_square},
		{.name = "Triangle 4px Polygon", .tst_fn = test_4px_triangle},
		{.name = "Triangle 6px Polygon", .tst_fn = test_6px_triangle},
		{.name = NULL}
	}
};
