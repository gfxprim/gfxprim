// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"

static int load_json(const char *path)
{
	gp_widget *layout = gp_widget_layout_json(path, NULL, NULL);

	if (!layout)
		return TST_UNTESTED;

	gp_widget_free(layout);

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "JSON loader benchmark",
	.tests = {
		{.name = "grid", .tst_fn = load_json,
		 .res_path = "data/json/grid.json",
		 .data = "grid.json",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
	         .bench_iter = 10000},

		{.name = "button types", .tst_fn = load_json,
		 .res_path = "data/json/button_types.json",
		 .data = "button_types.json",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
	         .bench_iter = 1000},

		{.name = "button type text", .tst_fn = load_json,
		 .res_path = "data/json/button_type_text.json",
		 .data = "button_type_text.json",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
	         .bench_iter = 5000},

		{.name = "tabs", .tst_fn = load_json,
		 .res_path = "data/json/tabs.json",
		 .data = "tabs.json",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
	         .bench_iter = 50000},

		{.name = NULL},
	}
};
