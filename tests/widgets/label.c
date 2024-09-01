// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

static int label_new_free(void)
{
	const char *text = "Label text";
	gp_widget *label;

	label = gp_widget_label_new(text, 0, 0);
	if (!label) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	if (strcmp(gp_widget_label_get(label), text)) {
		tst_msg("Wrong label text");
		return TST_FAILED;
	}

	if (gp_widget_label_tattr_get(label) != 0) {
		tst_msg("Wrong label text attribute %i expected 0",
		         gp_widget_label_tattr_get(label));
		return TST_FAILED;
	}

	if (gp_widget_label_width_get(label) != 0) {
		tst_msg("Wrong label width %u expected 0",
			gp_widget_label_width_get(label));
		return TST_FAILED;
	}

	gp_widget_free(label);

	return TST_PASSED;
}

static int label_printf_new_free(void)
{
	const char *text = "A very long label text!";
	gp_widget *label;

	label = gp_widget_label_printf_new(GP_TATTR_MONO, "%s", text);
	if (!label) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	if (strcmp(gp_widget_label_get(label), text)) {
		tst_msg("Wrong label text");
		return TST_FAILED;
	}

	if (gp_widget_label_tattr_get(label) != GP_TATTR_MONO) {
		tst_msg("Wrong label text attribute %i expected %i",
		         gp_widget_label_tattr_get(label), GP_TATTR_MONO);
		return TST_FAILED;
	}

	if (gp_widget_label_width_get(label) != 0) {
		tst_msg("Wrong label width %u expected 0",
			gp_widget_label_width_get(label));
		return TST_FAILED;
	}

	gp_widget_free(label);

	return TST_PASSED;
}

static int label_append_set_printf(void)
{
	gp_widget *label;
	const char *set_text = "Now something completely different";
	const char *print_text = "My hovercraft is full of eels";

	label = gp_widget_label_new("Hello ", 0, 0);
	if (!label) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_label_append(label, "world!");

	if (strcmp(gp_widget_label_get(label), "Hello world!")) {
		tst_msg("Wrong label text after append");
		return TST_FAILED;
	}

	gp_widget_label_set(label, set_text);

	if (strcmp(gp_widget_label_get(label), set_text)) {
		tst_msg("Wrong label text after set");
		return TST_FAILED;
	}

	gp_widget_label_printf(label, "%s", print_text);

	if (strcmp(gp_widget_label_get(label), print_text)) {
		tst_msg("Wrong label text after printf");
		return TST_FAILED;
	}

	if (gp_widget_label_tattr_get(label) != 0) {
		tst_msg("Wrong label text attribute %i expected 0",
		         gp_widget_label_tattr_get(label));
		return TST_FAILED;
	}

	if (gp_widget_label_width_get(label) != 0) {
		tst_msg("Wrong label width %u expected 0",
			gp_widget_label_width_get(label));
		return TST_FAILED;
	}

	gp_widget_free(label);

	return TST_PASSED;
}

static int label_tattr_set(void)
{
	gp_widget *label;

	label = gp_widget_label_new("Label text", 0, 0);
	if (!label) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	if (gp_widget_label_tattr_get(label) != 0) {
		tst_msg("Wrong label text attribute %i expected 0",
		         gp_widget_label_tattr_get(label));
		return TST_FAILED;
	}

	gp_widget_tattr attr = GP_TATTR_LARGE | GP_TATTR_BOLD;

	gp_widget_label_tattr_set(label, attr);

	if (gp_widget_label_tattr_get(label) != attr) {
		tst_msg("Wrong label text attribute %i expected %i",
		         gp_widget_label_tattr_get(label), attr);
		return TST_FAILED;
	}

	gp_widget_free(label);

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "label testsuite",
	.tests = {
		{.name = "label new free",
		 .tst_fn = label_new_free,
		 .flags = TST_CHECK_MALLOC},

		{.name = "label printf new free",
		 .tst_fn = label_printf_new_free,
		 .flags = TST_CHECK_MALLOC},

		{.name = "label append set printf",
		 .tst_fn = label_append_set_printf,
		 .flags = TST_CHECK_MALLOC},

		{.name = "label tattr set",
		 .tst_fn = label_tattr_set,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
