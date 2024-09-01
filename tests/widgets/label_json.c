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
	const char *text;
	gp_widget_tattr tattr;
	enum gp_widget_label_flags flags;
	unsigned int width;
};

static int label_json_load(struct tcase *t)
{
	gp_widget *label;

	label = gp_widget_from_json_str(t->json, NULL, NULL);
	if (!label) {
		tst_msg("Failed to load JSON");
		return TST_FAILED;
	}

	if (label->type != GP_WIDGET_LABEL) {
		tst_msg("Wrong widget type!");
		return TST_FAILED;
	}

	if (strcmp(t->text, gp_widget_label_get(label))) {
		tst_msg("Wrong label text!");
		return TST_FAILED;
	}

	if (t->flags != gp_widget_label_flags_get(label)) {
		tst_msg("Wrong flags expected %02x!", t->flags);
		return TST_FAILED;
	}

	if (t->tattr != gp_widget_label_tattr_get(label)) {
		tst_msg("Wrong label tattr!");
		return TST_FAILED;
	}

	if (t->width != gp_widget_label_width_get(label)) {
		tst_msg("Wrong width %u expected %i", gp_widget_label_width_get(label), t->width);
		return TST_FAILED;
	}

	gp_widget_free(label);

	return TST_PASSED;
}

static struct tcase label = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        "\"layout\": {\"type\": \"label\", \"text\": \"Label!\"}}",
	.text = "Label!",
};

static struct tcase label_bold = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        "\"layout\": {\"type\": \"label\", \"tattr\": \"bold\", \"text\": \"Label!\"}}",
	.text = "Label!",
	.tattr = GP_TATTR_BOLD,
};

static struct tcase label_frame = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        "\"layout\": {\"type\": \"label\", \"frame\": true, \"text\": \"Label!\"}}",
	.text = "Label!",
	.flags = GP_WIDGET_LABEL_FRAME,
};

static struct tcase label_align = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        "\"layout\": {\"type\": \"label\", \"tattr\": \"right\", \"text\": \"Label!\"}}",
	.text = "Label!",
	.tattr = GP_TATTR_RIGHT,
};

static struct tcase label_width = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        "\"layout\": {\"type\": \"label\", \"width\": 10, \"text\": \"Label!\"}}",
	.text = "Label!",
	.width = 10,
};

const struct tst_suite tst_suite = {
	.suite_name = "label JSON testsuite",
	.tests = {
		{.name = "label",
		 .tst_fn = label_json_load,
		 .data = &label,
		 .flags = TST_CHECK_MALLOC},

		{.name = "label bold",
		 .tst_fn = label_json_load,
		 .data = &label_bold,
		 .flags = TST_CHECK_MALLOC},

		{.name = "label frame",
		 .tst_fn = label_json_load,
		 .data = &label_frame,
		 .flags = TST_CHECK_MALLOC},

		{.name = "label align",
		 .tst_fn = label_json_load,
		 .data = &label_align,
		 .flags = TST_CHECK_MALLOC},

		{.name = "label width",
		 .tst_fn = label_json_load,
		 .data = &label_width,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
