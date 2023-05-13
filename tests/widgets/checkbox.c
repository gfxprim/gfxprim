// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

static int checkbox_new_free(void)
{
	gp_widget *chbox;

	chbox = gp_widget_checkbox_new("Checkbox", 0);
	if (!chbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_free(chbox);

	return TST_PASSED;
}

static int ev_handler(gp_widget_event *ev)
{
	int *flag = ev->self->priv;

	if (ev->type == GP_WIDGET_EVENT_WIDGET &&
	    ev->sub_type == 0) {
		tst_msg("Got checkbox event");
		*flag = 1;
	}

	return 0;
}

static int checkbox_event_set(void)
{
	int flag = 0;
	gp_widget *chbox;

	chbox = gp_widget_checkbox_new2(NULL, 0, ev_handler, &flag);

	if (chbox->checkbox->val != 0) {
		tst_msg("Wrong checkbox value");
		return TST_FAILED;
	}

	gp_widget_checkbox_set(chbox, 1);

	if (!flag) {
		tst_msg("Did not get event!");
		return TST_FAILED;
	}

	if (chbox->checkbox->val != 1) {
		tst_msg("Wrong checkbox value");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int checkbox_event_toggle(void)
{
	int flag = 0;
	gp_widget *chbox;

	chbox = gp_widget_checkbox_new2(NULL, 1, ev_handler, &flag);

	if (chbox->checkbox->val != 1) {
		tst_msg("Wrong checkbox value");
		return TST_FAILED;
	}

	gp_widget_checkbox_toggle(chbox);

	if (!flag) {
		tst_msg("Did not get event!");
		return TST_FAILED;
	}

	if (chbox->checkbox->val != 0) {
		tst_msg("Wrong checkbox value");
		return TST_FAILED;
	}

	flag = 0;
	gp_widget_checkbox_toggle(chbox);

	if (!flag) {
		tst_msg("Did not get event!");
		return TST_FAILED;
	}

	if (chbox->checkbox->val != 1) {
		tst_msg("Wrong checkbox value");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int checkbox_event_key(void)
{
	int flag = 0;
	gp_widget *chbox;

	chbox = gp_widget_checkbox_new2(NULL, 0, ev_handler, &flag);

	send_keypress(chbox, GP_KEY_ENTER);

	if (!flag) {
		tst_msg("Did not get event!");
		return TST_FAILED;
	}

	if (chbox->checkbox->val != 1) {
		tst_msg("Wrong checkbox value");
		return TST_FAILED;
	}

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "checkbox testsuite",
	.tests = {
		{.name = "checkbox new free",
		 .tst_fn = checkbox_new_free,
		 .flags = TST_CHECK_MALLOC},

		{.name = "checkbox event set",
		 .tst_fn = checkbox_event_set},

		{.name = "checkbox event toggle",
		 .tst_fn = checkbox_event_toggle},

		{.name = "checkbox event key",
		 .tst_fn = checkbox_event_key},

		{.name = NULL},
	}
};
