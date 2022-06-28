// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"

static int on_event(gp_widget_event *ev)
{
	int *ret = ev->ptr;
	*ret = ev->type;

	return 0;
}

static int app_no_on_event(void)
{
	int ret;

	ret = gp_app_send_event(GP_WIDGET_EVENT_FREE, 0);
	if (ret) {
		tst_msg("Non-zero returned with no handler!");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int app_event_free(void)
{
	int ret_ev_type = -1;

	gp_app_on_event_set(on_event);
	gp_app_send_event(GP_WIDGET_EVENT_FREE, &ret_ev_type);

	if (ret_ev_type != GP_WIDGET_EVENT_FREE) {
		tst_msg("GP_WIDGET_EVENT_FREE not send to a handler!");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int app_event_free_masked(void)
{
	int ret_ev_type = -1;

	gp_app_on_event_set(on_event);
	gp_app_event_mask(GP_WIDGET_EVENT_FREE);
	gp_app_send_event(GP_WIDGET_EVENT_FREE, &ret_ev_type);

	if (ret_ev_type != -1) {
		tst_msg("GP_WIDGET_EVENT_FREE not masked");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int app_event_input(void)
{
	int ret_ev_type = -1;

	gp_app_on_event_set(on_event);
	gp_app_send_event(GP_WIDGET_EVENT_INPUT, &ret_ev_type);

	if (ret_ev_type != -1) {
		tst_msg("GP_WIDGET_EVENT_INPUT not masked by default");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int app_event_input_unmasked(void)
{
	int ret_ev_type = -1;

	gp_app_on_event_set(on_event);
	gp_app_event_unmask(GP_WIDGET_EVENT_INPUT);
	gp_app_send_event(GP_WIDGET_EVENT_INPUT, &ret_ev_type);

	if (ret_ev_type != GP_WIDGET_EVENT_INPUT) {
		tst_msg("GP_WIDGET_EVENT_INPUT not unmasked");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "widget app events testsuite",
	.tests = {
		{.name = "app no on_event",
		 .tst_fn = app_no_on_event},

		{.name = "app event free",
		 .tst_fn = app_event_free},

		{.name = "app event free masked",
		 .tst_fn = app_event_free_masked},

		{.name = "app event input",
		 .tst_fn = app_event_input},

		{.name = "app event input unmasked",
		 .tst_fn = app_event_input_unmasked},

		{.name = NULL},
	}
};
