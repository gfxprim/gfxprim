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

	tbox = gp_widget_tbox_new("", 0, 10, 0, NULL, 0, NULL, NULL);
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

	if (ev->type == GP_WIDGET_EVENT_WIDGET &&
	    ev->sub_type == GP_WIDGET_TBOX_TRIGGER) {
		tst_msg("Got action event");
		*flag = 1;
	}

	return 0;
}

static int tbox_event_action(void)
{
	gp_widget *tbox;
	int flag = 0;

	tbox = gp_widget_tbox_new("", 0, 10, 0, NULL, 0, ev_handler, &flag);
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

	tbox = gp_widget_tbox_new("", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	type_string(tbox, "hello world");

	str = gp_widget_tbox_text(tbox);
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

	str = gp_widget_tbox_text(tbox);
	if (strcmp(str, "hell world")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	type_string(tbox, "ish");

	str = gp_widget_tbox_text(tbox);
	if (strcmp(str, "hellish world")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_tbox_clear(tbox);

	str = gp_widget_tbox_text(tbox);
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

	tbox = gp_widget_tbox_new("world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 5) {
		tst_msg("Wrong cursor position %zu expected 5", cur_pos);
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, -5, GP_SEEK_CUR);

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

	str = gp_widget_tbox_text(tbox);
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

	tbox = gp_widget_tbox_new("world", 0, 10, 0, NULL, 0, NULL, NULL);
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

	str = gp_widget_tbox_text(tbox);
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

	str = gp_widget_tbox_text(tbox);
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

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_del(tbox, 0, GP_SEEK_SET, 6);
	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 5) {
		tst_msg("Wrong cursor position %zu expected 5 '%s'", cur_pos, tbox->tbox->buf);
		return TST_FAILED;
	}

	str = gp_widget_tbox_text(tbox);
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

	str = gp_widget_tbox_text(tbox);
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

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
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

	str = gp_widget_tbox_text(tbox);
	if (strcmp(str, "666")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_free(tbox);

	return TST_SUCCESS;
}

static int tbox_set(void)
{
	gp_widget *tbox;
	const char *str;
	size_t cur_pos;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_set(tbox, "666");

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos != 3) {
		tst_msg("Wrong cursor position %zu expected 3", cur_pos);
		return TST_FAILED;
	}

	str = gp_widget_tbox_text(tbox);
	if (strcmp(str, "666")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_free(tbox);

	return TST_SUCCESS;
}

static int tbox_sel_all(void)
{
	gp_widget *tbox;
	size_t val;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_all(tbox);

	val = gp_widget_tbox_sel_off(tbox);
	if (val) {
		tst_msg("Wrong offset %zu after select all", val);
		return TST_FAILED;
	}

	val = gp_widget_tbox_sel_len(tbox);
	if (val != 11) {
		tst_msg("Wrong lenght %zu after select all", val);
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val != 11) {
		tst_msg("Wrong cursor %zu after select all", val);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tbox_sel_set(void)
{
	gp_widget *tbox;
	size_t val;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, -2, GP_SEEK_END, 2);

	val = gp_widget_tbox_sel_off(tbox);
	if (val != 9) {
		tst_msg("Wrong offset %zu after select all", val);
		return TST_FAILED;
	}

	val = gp_widget_tbox_sel_len(tbox);
	if (val != 2) {
		tst_msg("Wrong lenght %zu after select all", val);
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val != 11) {
		tst_msg("Wrong cursor %zu after select all", val);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tbox_sel_del(void)
{
	gp_widget *tbox;
	size_t val;
	const char *str;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Text selected after tbox was created!");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, -2, GP_SEEK_END, 2);

	gp_widget_tbox_sel_del(tbox);

	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Selection not cleared after delete!");
		return TST_FAILED;
	}

	str = gp_widget_tbox_text(tbox);
	if (strcmp(str, "hello wor")) {
		tst_msg("Wrong text '%s' after selection delete!", str);
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val != 9) {
		tst_msg("Cursor (%zu) not updated after selection delete!", val);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tbox_sel_keys_ascii(void)
{
	gp_widget *tbox;
	size_t val;
	const char *str;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, 0, GP_SEEK_SET, 5);

	type_string(tbox, "goodbye");

	str = gp_widget_tbox_text(tbox);
	if (strcmp(str, "goodbye world")) {
		tst_msg("Wrong text '%s' after selection delete!", str);
		return TST_FAILED;
	}

	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Selection not cleared after ascii input!");
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val != 7) {
		tst_msg("Wrong cursor position %zu after text input!", val);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tbox_sel_keys_clear(void)
{
	gp_widget *tbox;
	size_t val;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, 1, GP_SEEK_SET, 1);
	send_keypress(tbox, GP_KEY_LEFT, 0);
	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Selection not cleared by KEY_LEFT!");
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val != 1) {
		tst_msg("Wrong cursor %zu ater KEY_LEFT!", val);
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, 1, GP_SEEK_SET, 1);
	send_keypress(tbox, GP_KEY_RIGHT, 0);
	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Selection not cleared by KEY_RIGHT!");
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val != 2) {
		tst_msg("Wrong cursor %zu ater KEY_RIGHT!", val);
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, 1, GP_SEEK_SET, 1);
	send_keypress(tbox, GP_KEY_HOME, 0);
	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Selection not cleared by KEY_HOME!");
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val != 0) {
		tst_msg("Wrong cursor %zu ater KEY_HOME!", val);
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, 1, GP_SEEK_SET, 1);
	send_keypress(tbox, GP_KEY_END, 0);
	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Selection not cleared by KEY_END!");
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val != 11) {
		tst_msg("Wrong cursor %zu ater KEY_END!", val);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tbox_sel_key_del_backspace(int key)
{
	gp_widget *tbox;
	const char *str;
	size_t val;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, 2, GP_SEEK_SET, 7);
	send_keypress(tbox, key, 0);

	str = gp_widget_tbox_text(tbox);
	if (strcmp(str, "held")) {
		tst_msg("Wrong text '%s' after deleted selection!", str);
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val != 2) {
		tst_msg("Wrong cursor position %zu after deleted selection!", val);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tbox_sel_key_del(void)
{
	return tbox_sel_key_del_backspace(GP_KEY_DELETE);
}

static int tbox_sel_key_backspace(void)
{
	return tbox_sel_key_del_backspace(GP_KEY_BACKSPACE);
}

static int tbox_sel_key_left(void)
{
	gp_widget *tbox;
	size_t val;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, 2, GP_SEEK_SET);
	state_press(GP_KEY_LEFT_SHIFT);
	send_keypress(tbox, GP_KEY_LEFT, 0);

	val = gp_widget_tbox_sel_len(tbox);
	if (val != 1) {
		tst_msg("Wrong selection lenght %zu expected 1", val);
		return TST_FAILED;
	}

	val = gp_widget_tbox_sel_off(tbox);
	if (val != 1) {
		tst_msg("Wrong selection offset %zu expected 1", val);
		return TST_FAILED;
	}

	send_keypress(tbox, GP_KEY_RIGHT, 0);

	val = gp_widget_tbox_sel_len(tbox);
	if (val) {
		tst_msg("Selection should have been cleared!");
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val != 2) {
		tst_msg("Wrong cursor position %zu expected 2", val);
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

		{.name = "tbox cursor",
		 .tst_fn = tbox_cursor},

		{.name = "tbox insert",
		 .tst_fn = tbox_ins},

		{.name = "tbox delete",
		 .tst_fn = tbox_del},

		{.name = "tbox printf",
		 .tst_fn = tbox_printf},

		{.name = "tbox set",
		 .tst_fn = tbox_set},

		{.name = "tbox sel all",
		 .tst_fn = tbox_sel_all},

		{.name = "tbox sel set",
		 .tst_fn = tbox_sel_set},

		{.name = "tbox sel del",
		 .tst_fn = tbox_sel_del},

		{.name = "tbox sel keys ascii",
		 .tst_fn = tbox_sel_keys_ascii},

		{.name = "tbox sel keys clear",
		 .tst_fn = tbox_sel_keys_clear},

		{.name = "tbox sel key del",
		 .tst_fn = tbox_sel_key_del},

		{.name = "tbox sel key backspace",
		 .tst_fn = tbox_sel_key_backspace},

		{.name = "tbox sel key left",
		 .tst_fn = tbox_sel_key_left},

		{.name = NULL},
	}
};
