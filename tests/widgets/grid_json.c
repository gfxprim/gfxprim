// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

struct child {
	int type;
};

struct tcase {
	const char *json;
	unsigned int cols;
	unsigned int rows;
	int frame;
	int uniform;
	struct child children[];
};

static int grid_json_load(struct tcase *t)
{
	gp_widget *grid;

	grid = gp_widget_from_json_str(t->json, NULL);
	if (!grid) {
		tst_msg("Failed to load JSON");
		return TST_FAILED;
	}

	if (grid->type != GP_WIDGET_GRID) {
		tst_msg("Wrong widget type!");
		return TST_FAILED;
	}

	if (grid->grid->cols != t->cols) {
		tst_msg("Wrong number of columns %u, expected %u\n",
			grid->grid->cols, t->cols);
		return TST_FAILED;
	}

	if (grid->grid->rows != t->rows) {
		tst_msg("Wrong number of columns %u, expected %u\n",
			grid->grid->rows, t->rows);
		return TST_FAILED;
	}

	if (t->frame != !!grid->grid->frame) {
		tst_msg("Wrong frame flag expected %i", t->frame);
		return TST_FAILED;
	}

	if (t->uniform != !!grid->grid->uniform) {
		tst_msg("Wrong uniform flag expected %i", t->uniform);
		return TST_FAILED;
	}

	unsigned int col, row;

	for (col = 0; col < t->cols; col++) {
		for (row = 0; row < t->rows; row++) {
			gp_widget *child = gp_widget_grid_get(grid, col, row);
			struct child *c = &t->children[col * t->rows + row];

			if (c->type == -1) {
				if (child) {
					tst_msg("Expected NULL child at %ux%u", col, row);
					return TST_FAILED;
				}
			} else {
				if (!child) {
					tst_msg("Expected non NULL child at %ux%u", col, row);
					return TST_FAILED;
				}

				if (child->type != (unsigned int)c->type) {
					tst_msg("Wrong child widget type at %ux%u", col, row);
					return TST_FAILED;
				}
			}
		}
	}

	gp_widget_free(grid);

	return TST_SUCCESS;
}

static struct tcase grid_0x0 = {
	.json = "{\"version\": 1, \"cols\": 0, \"rows\": 0}",
	.cols = 0,
	.rows = 0,
};

static struct tcase grid_1x1_empty = {
	.json = "{\"version\": 1, \"widgets\": [{}]}",
	.cols = 1,
	.rows = 1,
	.children = {
		{.type = -1},
	}
};

static struct tcase grid_1x1_grid = {
	.json = "{\"version\": 1, \"widgets\": [{\"widgets\":[]}]}",
	.cols = 1,
	.rows = 1,
	.children = {
		{.type = GP_WIDGET_GRID},
	}
};

static struct tcase grid_1x1_2_children = {
	.json = "{\"version\": 1, \"widgets\": [{\"widgets\":[]}, {}]}",
	.cols = 1,
	.rows = 1,
	.children = {
		{.type = GP_WIDGET_GRID},
	}
};

static struct tcase grid_2x2 = {
	.json = "{\"version\": 1, \"cols\": 2, \"rows\": 2, \"widgets\": [\n"
	        " {\"widgets\":[{}]},\n"
		" {\"type\": \"button\", \"label\": \"ok\"},\n"
	        " {\"widgets\":[{}]},\n"
	        " {\"widgets\":[{}]}\n"
                "]}\n",
	.cols = 2,
	.rows = 2,
	.children = {
		{.type = GP_WIDGET_GRID},
		{.type = GP_WIDGET_BUTTON},
		{.type = GP_WIDGET_GRID},
		{.type = GP_WIDGET_GRID},
	}
};

static struct tcase grid_3x1 = {
	.json = "{\"version\": 1, \"cols\": 3, \"rows\": 1, \"widgets\": [{}, {}, {}]}",
	.cols = 3,
	.rows = 1,
	.children = {
		{.type = -1},
		{.type = -1},
		{.type = -1},
	}
};

static struct tcase grid_uniform = {
	.json = "{\"version\": 1, \"cols\": 1, \"rows\": 1, \"uniform\": true, \"widgets\": [{}]}",
	.cols = 1,
	.rows = 1,
	.uniform = 1,
	.children = {
		{.type = -1},
	}
};

static struct tcase grid_frame = {
	.json = "{\"version\": 1, \"cols\": 1, \"rows\": 1, \"frame\": true, \"widgets\": [{}]}",
	.cols = 1,
	.rows = 1,
	.frame = 1,
	.children = {
		{.type = -1},
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "grid JSON testsuite",
	.tests = {
		{.name = "grid 0x0",
		 .tst_fn = grid_json_load,
		 .data = &grid_0x0,
		 .flags = TST_CHECK_MALLOC},

		{.name = "grid 1x1 empty",
		 .tst_fn = grid_json_load,
		 .data = &grid_1x1_empty,
		 .flags = TST_CHECK_MALLOC},

		{.name = "grid 1x1 grid",
		 .tst_fn = grid_json_load,
		 .data = &grid_1x1_grid,
		 .flags = TST_CHECK_MALLOC},

		{.name = "grid 1x1 2 children",
		 .tst_fn = grid_json_load,
		 .data = &grid_1x1_2_children,
		 .flags = TST_CHECK_MALLOC},

		{.name = "grid 2x2",
		 .tst_fn = grid_json_load,
		 .data = &grid_2x2,
		 .flags = TST_CHECK_MALLOC},

		{.name = "grid 3x1",
		 .tst_fn = grid_json_load,
		 .data = &grid_3x1,
		 .flags = TST_CHECK_MALLOC},

		{.name = "grid uniform",
		 .tst_fn = grid_json_load,
		 .data = &grid_uniform,
		 .flags = TST_CHECK_MALLOC},

		{.name = "grid frame",
		 .tst_fn = grid_json_load,
		 .data = &grid_frame,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
