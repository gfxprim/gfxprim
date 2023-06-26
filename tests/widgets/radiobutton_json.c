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
	size_t cnt;
	size_t sel;
	const char * const *choices;
};

static int radiobutton_json_load(struct tcase *t)
{
	gp_widget *radiobutton;
	size_t i;

	radiobutton = gp_widget_from_json_str(t->json, NULL, NULL);
	if (!radiobutton) {
		tst_msg("Failed to load JSON");
		return TST_FAILED;
	}

	if (radiobutton->type != GP_WIDGET_RADIOBUTTON) {
		tst_msg("Wrong widget type!");
		return TST_FAILED;
	}

	if (t->sel != radiobutton->choice->sel) {
		tst_msg("Wrong choice selected!");
		return TST_FAILED;
	}

	if (t->cnt != gp_widget_choice_cnt_get(radiobutton)) {
		tst_msg("Wrong choice count!");
		return TST_FAILED;
	}

	for (i = 0; i < t->cnt; i++) {
		const char *val = gp_widget_choice_name_get(radiobutton, i);

		if (strcmp(val, t->choices[i])) {
			tst_msg("Wrong choice value at %zu", i);
			return TST_FAILED;
		}
	}

	gp_widget_free(radiobutton);

	return TST_PASSED;
}

static int radiobutton_json_fail(struct tcase *t)
{
	gp_widget *radiobutton;

	radiobutton = gp_widget_from_json_str(t->json, NULL, NULL);
	if (radiobutton) {
		tst_msg("Loaded broken JSON");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static const char *get_choice(gp_widget *self, size_t idx)
{
	(void) self;

	switch (idx) {
	case 0:
		return "a";
	case 1:
		return "b";
	default:
		return NULL;
	}
}

static void set(gp_widget *self, size_t val)
{
	self->choice->sel = val;
}

static size_t get(gp_widget *self, enum gp_widget_choice_op op)
{
	switch (op) {
	case GP_WIDGET_CHOICE_OP_SEL:
		return self->choice->sel;
	case GP_WIDGET_CHOICE_OP_CNT:
		return 2;
	}

	return 0;
}

const gp_widget_choice_desc choice_desc = {
	.ops = &(gp_widget_choice_ops) {
		.get_choice = get_choice,
		.get = get,
		.set = set
	}
};

static struct tcase radiobutton = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"radiobutton\", \"choices\": [\"a\", \"b\"]}}",
	.cnt = 2,
	.choices = (const char *const []) {
		"a",
		"b",
	}
};

static struct tcase sel_radiobutton = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"radiobutton\", \"choices\": [\"a\", \"b\"], \"selected\": 1}}",
	.cnt = 2,
	.sel = 1,
	.choices = (const char *const []) {
		"a",
		"b",
	}
};

static struct tcase ops_radiobutton = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"radiobutton\", \"desc\": \"choice_desc\"}}",
	.cnt = 2,
	.sel = 0,
	.choices = (const char *const []) {
		"a",
		"b",
	}
};

static struct tcase empty_radiobutton = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"radiobutton\"}}"
};

static struct tcase missing_ops_radiobutton = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"radiobutton\", \"ops\": \"does_not_exist\"}}"
};

const struct tst_suite tst_suite = {
	.suite_name = "radiobutton JSON testsuite",
	.tests = {
		{.name = "radiobutton",
		 .tst_fn = radiobutton_json_load,
		 .data = &radiobutton,
		 .flags = TST_CHECK_MALLOC},

		{.name = "sel radiobutton",
		 .tst_fn = radiobutton_json_load,
		 .data = &sel_radiobutton,
		 .flags = TST_CHECK_MALLOC},

		{.name = "ops radiobutton",
		 .tst_fn = radiobutton_json_load,
		 .data = &ops_radiobutton,
		 .flags = TST_CHECK_MALLOC},

		{.name = "empty radiobutton",
		 .tst_fn = radiobutton_json_fail,
		 .data = &empty_radiobutton,
		 .flags = TST_CHECK_MALLOC},

		{.name = "missing ops radiobutton",
		 .tst_fn = radiobutton_json_fail,
		 .data = &missing_ops_radiobutton,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
