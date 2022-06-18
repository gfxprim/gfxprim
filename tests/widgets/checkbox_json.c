// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

struct tcase {
	const char *json;
	const char *label;
	int val;
};

static int checkbox_json_load(struct tcase *t)
{
	gp_widget *checkbox;

	checkbox = gp_widget_from_json_str(t->json, NULL, NULL);
	if (!checkbox) {
		tst_msg("Failed to load JSON");
		return TST_FAILED;
	}

	if (checkbox->type != GP_WIDGET_CHECKBOX) {
		tst_msg("Wrong widget type!");
		return TST_FAILED;
	}

	if (t->label) {
		if (strcmp(t->label, checkbox->checkbox->label)) {
			tst_msg("Wrong checkbox label!");
			return TST_FAILED;
		}
	} else {
		if (checkbox->checkbox->label) {
			tst_msg("Non-NULL label!");
			return TST_FAILED;
		}
	}

	if (t->val != checkbox->checkbox->val) {
		tst_msg("Wrong checkbox type!");
		return TST_FAILED;
	}

	gp_widget_free(checkbox);

	return TST_SUCCESS;
}

static struct tcase checkbox = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"checkbox\"}}",
};

static struct tcase set_checkbox = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"checkbox\", \"set\": true}}",
	.val = 1,
};

static struct tcase labeled_checkbox = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"checkbox\", \"label\": \"Label!\"}}",
	.label = "Label!",
};

const struct tst_suite tst_suite = {
	.suite_name = "checkbox JSON testsuite",
	.tests = {
		{.name = "checkbox",
		 .tst_fn = checkbox_json_load,
		 .data = &checkbox,
		 .flags = TST_CHECK_MALLOC},

		{.name = "checkbox",
		 .tst_fn = checkbox_json_load,
		 .data = &set_checkbox,
		 .flags = TST_CHECK_MALLOC},

		{.name = "checkbox labeled",
		 .tst_fn = checkbox_json_load,
		 .data = &labeled_checkbox,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
