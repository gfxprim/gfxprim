// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

static int button_new_free(void)
{
	gp_widget *button;

	button = gp_widget_button_new("Button", 0);
	if (!button) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_free(button);

	return TST_SUCCESS;
}

static int ev_handler(gp_widget_event *ev)
{
	int *flag = ev->self->priv;

	if (ev->type == GP_WIDGET_EVENT_WIDGET &&
	    ev->sub_type == 0) {
		tst_msg("Got button event");
		*flag = 1;
	}

	return 0;
}

static int button_event_key(void)
{
	int flag = 0;
	gp_widget *button;

	button = gp_widget_button_new2("Button", 0, ev_handler, &flag);

	send_keypress(button, GP_KEY_ENTER);

	if (!flag) {
		tst_msg("Did not get event!");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "button testsuite",
	.tests = {
		{.name = "button new free",
		 .tst_fn = button_new_free,
		 .flags = TST_CHECK_MALLOC},

		{.name = "button key",
		 .tst_fn = button_event_key},

		{.name = NULL},
	}
};
