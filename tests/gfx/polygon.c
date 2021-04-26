// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <gfx/gp_polygon.h>

#include <gfx/gp_line.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* polygon description */
	unsigned int edge_count;
	gp_coord edges[20];

	/* expected result */
	gp_size w, h;
	const char pixmap[];
};

static int test_polygon(struct testcase *t)
{
	gp_pixmap *c;
	int err;

	c = pixmap_alloc_canary(t->w, t->h, GP_PIXEL_G8);

	if (!c) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	/* zero the pixels buffer */
	memset(c->pixels, 0, c->w * c->h);

	gp_fill_polygon(c, t->edge_count, t->edges, 1);
/*
	int lx = t->edges[2*t->edge_count-2];
	int ly = t->edges[2*t->edge_count-1];

	for (int i = 0; i < t->edge_count; i++) {
		int x = t->edges[2*i];
		int y = t->edges[2*i+1];

		gp_line(c, lx, ly, x, y, 1);
		lx=x;
		ly=y;
	}
*/
	err = compare_buffers(t->pixmap, c) || check_canary(c);

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
		1, 1,
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

struct testcase testcase_triangle_1 = {
	.edge_count = 3,
	.edges = {
		1, 4,
		8, 2,
		2, 8,
	},
	.w = 10,
	.h = 10,
	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

struct testcase testcase_bow_4edges_7px = {
	.edge_count = 4,
	.edges = {
		1, 1,
		1, 3,
		3, 1,
		3, 3,
	},
	.w = 5,
	.h = 5,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 0, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 0, 1, 0,
		0, 0, 0, 0, 0,
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "Polygon Testsuite",
	.tests = {
		{.name = "1 Edge Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_1_edge},

		{.name = "5 Edges 1px Polygon",
		.tst_fn = test_polygon,
		.data = &testcase_5_edges_1px},

		{.name = "Vertical Line 3px Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_line_vert_3px},

		{.name = "Horizontal Line 3px Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_line_horiz_3px},

		{.name = "Line 3px Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_line_3px},

		{.name = "Line 4px Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_line_4px},

		{.name = "2x2 Square Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_2x2_square},

		{.name = "3x3 Square Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_3x3_square},

		{.name = "4x4 Square Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_4x4_square},

		{.name = "Triangle 4px Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_4px_triangle},

		{.name = "Triangle 6px Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_6px_triangle},

		{.name = "Triangle 1. Polygon",
		 .tst_fn = test_polygon,
		 .data = &testcase_triangle_1},

		{.name = "Bow 4edges 7px",
		 .tst_fn = test_polygon,
		 .data = &testcase_bow_4edges_7px},

		{.name = NULL}
	}
};
