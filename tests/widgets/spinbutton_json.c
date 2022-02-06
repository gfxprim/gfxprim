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

static int spinbutton_json_load(struct tcase *t)
{
	gp_widget *spinbutton;
	size_t i;

	spinbutton = gp_widget_from_json_str(t->json, NULL, NULL);
	if (!spinbutton) {
		tst_msg("Failed to load JSON");
		return TST_FAILED;
	}

	if (spinbutton->type != GP_WIDGET_SPINBUTTON) {
		tst_msg("Wrong widget type!");
		return TST_FAILED;
	}

	if (t->sel != spinbutton->choice->sel) {
		tst_msg("Wrong choice selected!");
		return TST_FAILED;
	}

	if (t->cnt != gp_widget_choice_cnt_get(spinbutton)) {
		tst_msg("Wrong choice count!");
		return TST_FAILED;
	}

	for (i = 0; i < t->cnt; i++) {
		const char *val = gp_widget_choice_name_get(spinbutton, i);

		if (strcmp(val, t->choices[i])) {
			tst_msg("Wrong choice value at %zu", i);
			return TST_FAILED;
		}
	}

	gp_widget_free(spinbutton);

	return TST_SUCCESS;
}

static int spinbutton_json_fail(struct tcase *t)
{
	gp_widget *spinbutton;

	spinbutton = gp_widget_from_json_str(t->json, NULL, NULL);
	if (spinbutton) {
		tst_msg("Loaded broken JSON");
		return TST_FAILED;
	}

	return TST_SUCCESS;
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

static void set(gp_widget *self, enum gp_widget_choice_op op, size_t val)
{
	if (op != GP_WIDGET_CHOICE_OP_SEL)
		return;

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

struct gp_widget_choice_ops choice_ops = {
	.get_choice = get_choice,
	.get = get,
	.set = set
};

static struct tcase spinbutton = {
	.json = "{\"version\": 1, \"type\": \"spinbutton\", \"choices\": [\"a\", \"b\"]}",
	.cnt = 2,
	.choices = (const char *const []) {
		"a",
		"b",
	}
};

static struct tcase sel_spinbutton = {
	.json = "{\"version\": 1, \"type\": \"spinbutton\", \"choices\": [\"a\", \"b\"], \"selected\": 1}",
	.cnt = 2,
	.sel = 1,
	.choices = (const char *const []) {
		"a",
		"b",
	}
};

static struct tcase ops_spinbutton = {
	.json = "{\"version\": 1, \"type\": \"spinbutton\", \"ops\": \"choice_ops\"}",
	.cnt = 2,
	.sel = 0,
	.choices = (const char *const []) {
		"a",
		"b",
	}
};

static struct tcase empty_spinbutton = {
	.json = "{\"version\": 1, \"type\": \"spinbutton\"}"
};

static struct tcase missing_ops_spinbutton = {
	.json = "{\"version\": 1, \"type\": \"spinbutton\", \"ops\": \"does_not_exist\"}"
};

const struct tst_suite tst_suite = {
	.suite_name = "spinbutton JSON testsuite",
	.tests = {
		{.name = "spinbutton",
		 .tst_fn = spinbutton_json_load,
		 .data = &spinbutton,
		 .flags = TST_CHECK_MALLOC},

		{.name = "sel spinbutton",
		 .tst_fn = spinbutton_json_load,
		 .data = &sel_spinbutton,
		 .flags = TST_CHECK_MALLOC},

		{.name = "ops spinbutton",
		 .tst_fn = spinbutton_json_load,
		 .data = &ops_spinbutton,
		 .flags = TST_CHECK_MALLOC},

		{.name = "empty spinbutton",
		 .tst_fn = spinbutton_json_fail,
		 .data = &empty_spinbutton,
		 .flags = TST_CHECK_MALLOC},

		{.name = "missing ops spinbutton",
		 .tst_fn = spinbutton_json_fail,
		 .data = &missing_ops_spinbutton,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
