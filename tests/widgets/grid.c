// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

static int grid_new_free(void)
{
	gp_widget *grid, *ret;

	grid = gp_widget_grid_new(1, 1, 0);
	if (!grid) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	ret = gp_widget_grid_get(grid, 0, 0);
	if (ret) {
		tst_msg("gp_widget_grid_get(0,0) returned %p", ret);
		return TST_FAILED;
	}

	gp_widget_free(grid);

	return TST_SUCCESS;
}

static int grid_free_children(void)
{
	gp_widget *grid;
	gp_widget *label;

	grid = gp_widget_grid_new(1, 1, 0);
	label = gp_widget_label_new("Label", 0, 0);
	if (!grid || !label) {
		free(grid);
		free(label);
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(grid, 0, 0, label);

	gp_widget_free(grid);

	return TST_SUCCESS;
}

static int grid_ins_rem_rows(void)
{
	gp_widget *grid, *ret, *l;
	unsigned int c, r;

	grid = gp_widget_grid_new(2, 2, 0);
	if (!grid) {
		free(grid);
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	/* Fill the grid with widgets */
	for (c = 0; c < 2; c++) {
		for (r = 0; r < 2; r++) {
			l = gp_widget_label_printf_new(0, "Label %ux%u", c, r);

			ret = gp_widget_grid_put(grid, c, r, l);

			if (ret) {
				tst_msg("gp_widget_grid_put(%u,%u) returned %p", c, r, ret);
				return TST_FAILED;
			}
		}
	}

	/* Insert row and check if everything is fine */
	gp_widget_grid_rows_ins(grid, 1, 1);

	if (grid->grid->rows != 3) {
		tst_msg("Wrong number of rows after insert %u expected 3",
		        grid->grid->rows);
		return TST_FAILED;
	}

	for (c = 0; c < 2; c++) {
		ret = gp_widget_grid_get(grid, c, 1);

		if (ret) {
			tst_msg("Non-NULL pointer in inserted row %ux1 %p",
			        c, ret);
			return TST_FAILED;
		}
	}

	/* Insert widget into the newly added row */
	l = gp_widget_label_new("Label in new row", 0, 0);

	ret = gp_widget_grid_put(grid, 0, 1, l);
	if (ret) {
		tst_msg("gp_widget_grid_put(0, 1) returned %p", ret);
		return TST_FAILED;
	}

	/* Delete the inserted row & free the widget put into the row */
	gp_widget_grid_rows_del(grid, 1, 1);

	if (grid->grid->rows != 2) {
		tst_msg("Wrong number of rows after insert %u expected 2",
		        grid->grid->rows);
		return TST_FAILED;
	}

	/* Check that the grid is correct */
	for (c = 0; c < 2; c++) {
		for (r = 0; r < 2; r++) {
			char buf[64];
			l = gp_widget_grid_get(grid, c, r);

			if (!l) {
				tst_msg("NULL widget at %u,%u", c, r);
				return TST_FAILED;
			}

			snprintf(buf, sizeof(buf), "Label %ux%u", c, r);

			if (strcmp(l->label->text, buf)) {
				tst_msg("Wrong widget at %u,%u", c, r);
				return TST_FAILED;
			}
		}
	}

	gp_widget_free(grid);

	return TST_SUCCESS;
}

static int grid_put_get_rem_del(void)
{
	gp_widget *grid, *label, *ret;

	grid = gp_widget_grid_new(1, 1, 0);
	label = gp_widget_label_new("Label", 0, 0);
	if (!grid || !label) {
		free(grid);
		free(label);
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	/* attempt to crash by row/col out of the grid */
	gp_widget_grid_put(grid, 1, 0, label);
	gp_widget_grid_put(grid, 0, 1, label);
	gp_widget_grid_get(grid, 2, 0);
	gp_widget_grid_rem(grid, 0, 2);
	gp_widget_grid_del(grid, 2, 2);

	if (gp_widget_grid_get(grid, 0, 0)) {
		tst_msg("Something got inserted!");
		return TST_FAILED;
	}

	ret = gp_widget_grid_put(grid, 0, 0, label);
	if (ret) {
		tst_msg("Non NULL returned by initial put");
		return TST_FAILED;
	}

	ret = gp_widget_grid_get(grid, 0, 0);
	if (ret != label) {
		tst_msg("Wrong widget inserted!?");
		return TST_FAILED;
	}

	if (label->parent != grid) {
		tst_msg("Label parent not set!");
		return TST_FAILED;
	}

	ret = gp_widget_grid_rem(grid, 0, 0);
	if (ret != label) {
		tst_msg("Wrong pointer returned by rem");
		return TST_FAILED;
	}

	if (label->parent) {
		tst_msg("Parent pointer not cleared");
		return TST_FAILED;
	}

	ret = gp_widget_grid_put(grid, 0, 0, label);
	if (ret) {
		tst_msg("Non NULL returned by second put");
		return TST_FAILED;
	}

	/* make sure delete frees the widget */
	gp_widget_grid_del(grid, 0, 0);

	gp_widget_free(grid);

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "grid testsuite",
	.tests = {
		{.name = "grid new free",
		 .tst_fn = grid_new_free,
		 .flags = TST_CHECK_MALLOC},

		{.name = "grid free children",
		 .tst_fn = grid_free_children,
		 .flags = TST_CHECK_MALLOC},

		{.name = "grid ins rem rows",
		 .tst_fn = grid_ins_rem_rows,
		 .flags = TST_CHECK_MALLOC},

		{.name = "grid put get rem del",
		 .tst_fn = grid_put_get_rem_del,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
