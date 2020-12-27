// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

static int tbox_new(void)
{
	gp_widget *tbox;

	tbox = gp_widget_tbox_new("", 10, NULL, NULL, NULL, 0);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_free(tbox);

	return TST_SUCCESS;
}

static int ev_handler(gp_widget_event *ev)
{
	int *flag = ev->self->priv;

	if (ev->type == GP_WIDGET_EVENT_ACTION) {
		tst_msg("Got action event");
		*flag = 1;
	}

	return 0;
}

static int tbox_event_action(void)
{
	gp_widget *tbox;
	int flag = 0;

	tbox = gp_widget_tbox_new("", 10, NULL, ev_handler, &flag, 0);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	send_keypress(tbox, GP_KEY_ENTER, 0);

	if (flag)
		return TST_SUCCESS;

	return TST_FAILED;
}

static int tbox_typing(void)
{
	gp_widget *tbox;
	const char *str;

	tbox = gp_widget_tbox_new("", 10, NULL, NULL, NULL, 0);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	type_string(tbox, "hello world");

	str = gp_widget_tbox_str(tbox);
	if (strcmp(str, "hello world")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	send_keypress(tbox, GP_KEY_LEFT, 0);
	send_keypress(tbox, GP_KEY_LEFT, 0);
	send_keypress(tbox, GP_KEY_LEFT, 0);
	send_keypress(tbox, GP_KEY_LEFT, 0);
	send_keypress(tbox, GP_KEY_LEFT, 0);
	send_keypress(tbox, GP_KEY_LEFT, 0);
	send_keypress(tbox, GP_KEY_BACKSPACE, 0);

	str = gp_widget_tbox_str(tbox);
	if (strcmp(str, "hell world")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	type_string(tbox, "ish");

	str = gp_widget_tbox_str(tbox);
	if (strcmp(str, "hellish world")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_tbox_clear(tbox);

	str = gp_widget_tbox_str(tbox);
	if (strcmp(str, "")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "tbox testsuite",
	.tests = {
		{.name = "tbox new free",
		 .tst_fn = tbox_new,
		 .flags = TST_CHECK_MALLOC},

		{.name = "tbox event action",
		 .tst_fn = tbox_event_action},

		{.name = "tbox typing",
		 .tst_fn = tbox_typing},

		{.name = NULL},
	}
};
