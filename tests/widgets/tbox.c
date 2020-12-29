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
	size_t cur_pos;

	tbox = gp_widget_tbox_new("", 10, NULL, NULL, NULL, 0);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 0) {
		tst_msg("Wrong cursor position %zu expected 0", cur_pos);
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

	gp_widget_free(tbox);

	return TST_SUCCESS;
}

static int tbox_cursor(void)
{
	gp_widget *tbox;
	const char *str;
	size_t cur_pos;

	tbox = gp_widget_tbox_new("world", 10, NULL, NULL, NULL, 0);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 5) {
		tst_msg("Wrong cursor position %zu expected 5", cur_pos);
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, -6, GP_SEEK_CUR);

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 0) {
		tst_msg("Wrong cursor position %zu expected 0", cur_pos);
		return TST_FAILED;
	}

	type_string(tbox, "hello ");

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 6) {
		tst_msg("Wrong cursor position %zu expected 6", cur_pos);
		return TST_FAILED;
	}

	str = gp_widget_tbox_str(tbox);
	if (strcmp(str, "hello world")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_free(tbox);

	return TST_SUCCESS;
}

static int tbox_ins(void)
{
	gp_widget *tbox;
	const char *str;
	size_t cur_pos;

	tbox = gp_widget_tbox_new("world", 10, NULL, NULL, NULL, 0);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_ins(tbox, 0, GP_SEEK_SET, "hello ");
	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 11) {
		tst_msg("Wrong cursor position %zu expected 11", cur_pos);
		return TST_FAILED;
	}

	str = gp_widget_tbox_str(tbox);
	if (strcmp(str, "hello world")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_tbox_ins(tbox, 0, GP_SEEK_END, "s");
	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 12) {
		tst_msg("Wrong cursor position %zu expected 12", cur_pos);
		return TST_FAILED;
	}

	str = gp_widget_tbox_str(tbox);
	if (strcmp(str, "hello worlds")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, -1, GP_SEEK_CUR);
	gp_widget_tbox_ins(tbox, 0, GP_SEEK_END, " ");

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 11) {
		tst_msg("Wrong cursor position %zu expected 11", cur_pos);
		return TST_FAILED;
	}

	gp_widget_free(tbox);

	return TST_SUCCESS;
}

static int tbox_del(void)
{
	gp_widget *tbox;
	const char *str;
	size_t cur_pos;

	tbox = gp_widget_tbox_new("hello world", 10, NULL, NULL, NULL, 0);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_del(tbox, 0, GP_SEEK_SET, 6);
	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 5) {
		tst_msg("Wrong cursor position %zu expected 11", cur_pos);
		return TST_FAILED;
	}

	str = gp_widget_tbox_str(tbox);
	if (strcmp(str, "world")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, 2, GP_SEEK_SET);
	gp_widget_tbox_del(tbox, 0, GP_SEEK_SET, 100);

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 0) {
		tst_msg("Wrong cursor position %zu expected 0", cur_pos);
		return TST_FAILED;
	}

	str = gp_widget_tbox_str(tbox);
	if (strcmp(str, "")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_free(tbox);

	return TST_SUCCESS;
}

static int tbox_printf(void)
{
	gp_widget *tbox;
	const char *str;
	size_t cur_pos;

	tbox = gp_widget_tbox_new("hello world", 10, NULL, NULL, NULL, 0);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_printf(tbox, "%i", 666);

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 3) {
		tst_msg("Wrong cursor position %zu expected 3", cur_pos);
		return TST_FAILED;
	}

	str = gp_widget_tbox_str(tbox);
	if (strcmp(str, "666")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_free(tbox);

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

		{.name = "tbox cursor",
		 .tst_fn = tbox_cursor},

		{.name = "tbox insert",
		 .tst_fn = tbox_ins},

		{.name = "tbox delete",
		 .tst_fn = tbox_del},

		{.name = "tbox printf",
		 .tst_fn = tbox_printf},

		{.name = NULL},
	}
};
