// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2020-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

static int tbox_new(void)
{
	gp_widget *tbox;
	gp_utf8_pos cur_pos;

	tbox = gp_widget_tbox_new("", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos.chars || cur_pos.bytes) {
		tst_msg("Wrong cursor position %zu %zu expected 0",
		        cur_pos.chars, cur_pos.bytes);
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

/*
 * Checks that pressing enter triggers widget event handler.
 */
static int tbox_event_action(void)
{
	gp_widget *tbox;
	int flag = 0;

	tbox = gp_widget_tbox_new("", 0, 10, 0, NULL, 0, ev_handler, &flag);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	send_keypress(tbox, GP_KEY_ENTER);

	if (flag)
		return TST_SUCCESS;

	return TST_FAILED;
}

/*
 * Check that typing inserts text and backspace and delete removes it.
 */
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

	send_keypress(tbox, GP_KEY_LEFT);
	send_keypress(tbox, GP_KEY_LEFT);
	send_keypress(tbox, GP_KEY_LEFT);
	send_keypress(tbox, GP_KEY_LEFT);
	send_keypress(tbox, GP_KEY_LEFT);
	send_keypress(tbox, GP_KEY_LEFT);
	send_keypress(tbox, GP_KEY_BACKSPACE);

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

	send_keypress(tbox, GP_KEY_DELETE);

	str = gp_widget_tbox_text(tbox);
	if (strcmp(str, "hellishworld")) {
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

/*
 * Checks that cursor is set properly by gp_widget_tbox_cursor_set()
 */
static int tbox_cursor(void)
{
	gp_widget *tbox;
	const char *str;
	gp_utf8_pos cur_pos;

	tbox = gp_widget_tbox_new("world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos.chars != 5 || cur_pos.bytes != 5) {
		tst_msg("Wrong cursor position %zu %zu expected 5",
			cur_pos.chars, cur_pos.bytes);
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, -5, GP_SEEK_CUR);

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos.chars || cur_pos.bytes) {
		tst_msg("Wrong cursor position %zu %zu expected 0",
		        cur_pos.chars, cur_pos.bytes);
		return TST_FAILED;
	}

	type_string(tbox, "hello ");

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos.chars != 6 || cur_pos.bytes != 6) {
		tst_msg("Wrong cursor position %zu %zu expected 6",
		        cur_pos.chars, cur_pos.bytes);
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

/*
 * Check that text is inserted properly by gp_widget_tbox_ins()
 */
static int tbox_ins(void)
{
	gp_widget *tbox;
	const char *str;
	gp_utf8_pos cur_pos;

	tbox = gp_widget_tbox_new("world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_ins(tbox, 0, GP_SEEK_SET, "hello ");
	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos.chars != 11 || cur_pos.bytes != 11) {
		tst_msg("Wrong cursor position %zu %zu expected 11",
		        cur_pos.chars, cur_pos.bytes);
		return TST_FAILED;
	}

	str = gp_widget_tbox_text(tbox);
	if (strcmp(str, "hello world")) {
		tst_msg("Got wrong string: '%s'", str);
		return TST_FAILED;
	}

	gp_widget_tbox_ins(tbox, 0, GP_SEEK_END, "s");
	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos.chars != 12 || cur_pos.bytes != 12) {
		tst_msg("Wrong cursor position %zu %zu expected 12",
		        cur_pos.chars, cur_pos.bytes);
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
	if (cur_pos.chars != 11 || cur_pos.bytes != 11) {
		tst_msg("Wrong cursor position %zu %zu expected 11",
		        cur_pos.chars, cur_pos.bytes);
		return TST_FAILED;
	}

	gp_widget_free(tbox);

	return TST_SUCCESS;
}

/*
 * Checks that text is deleted properly by gp_widget_tbox_del()
 */
static int tbox_del(void)
{
	gp_widget *tbox;
	const char *str;
	gp_utf8_pos cur_pos;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_del(tbox, 0, GP_SEEK_SET, 6);
	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos.chars != 5 || cur_pos.bytes != 5) {
		tst_msg("Wrong cursor position %zu %zu expected 5 '%s'",
		        cur_pos.chars, cur_pos.bytes, tbox->tbox->buf);
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
	if (cur_pos.chars || cur_pos.bytes) {
		tst_msg("Wrong cursor position %zu %zu expected 0",
		        cur_pos.chars, cur_pos.bytes);
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

/*
 * Checks that buffer is replaced properly by gp_widget_tbox_printf().
 */
static int tbox_printf(void)
{
	gp_widget *tbox;
	const char *str;
	gp_utf8_pos cur_pos;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_printf(tbox, "%i", 666);

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos.chars != 3 || cur_pos.bytes != 3) {
		tst_msg("Wrong cursor position %zu %zu expected 3",
		        cur_pos.chars, cur_pos.bytes);
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
	gp_utf8_pos cur_pos;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_set(tbox, "666");

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos.chars != 3 || cur_pos.bytes != 3) {
		tst_msg("Wrong cursor position %zu %zu expected 3",
		        cur_pos.chars, cur_pos.bytes);
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
	gp_utf8_pos cur_pos;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_all(tbox);

	cur_pos = gp_widget_tbox_sel_off(tbox);
	if (cur_pos.chars || cur_pos.bytes) {
		tst_msg("Wrong offset %zu %zu after select all",
		        cur_pos.chars, cur_pos.bytes);
		return TST_FAILED;
	}

	cur_pos = gp_widget_tbox_sel_len(tbox);
	if (cur_pos.chars != 11 || cur_pos.bytes != 11) {
		tst_msg("Wrong lenght %zu %zu after select all",
		        cur_pos.chars, cur_pos.bytes);
		return TST_FAILED;
	}

	cur_pos = gp_widget_tbox_cursor_get(tbox);
	if (cur_pos.chars != 11 || cur_pos.bytes != 11) {
		tst_msg("Wrong cursor %zu %zu after select all",
		        cur_pos.chars, cur_pos.bytes);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tbox_sel_set(void)
{
	gp_widget *tbox;
	gp_utf8_pos val;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, -2, GP_SEEK_END, 2);

	val = gp_widget_tbox_sel_off(tbox);
	if (val.chars != 9 || val.bytes != 9) {
		tst_msg("Wrong offset %zu %zu after selection",
			val.chars, val.bytes);
		return TST_FAILED;
	}

	val = gp_widget_tbox_sel_len(tbox);
	if (val.chars != 2 || val.bytes != 2) {
		tst_msg("Wrong lenght %zu %zu after selection",
			val.chars, val.bytes);
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val.chars != 11 || val.bytes != 11) {
		tst_msg("Wrong cursor %zu %zu after selection",
			val.chars, val.bytes);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

/*
 * Test for gp_widget_tbox_sel_del() function.
 */
static int tbox_sel_del(void)
{
	gp_widget *tbox;
	gp_utf8_pos val;
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
	if (val.chars != 9 || val.bytes != 9) {
		tst_msg("Cursor (%zu %zu) not updated after selection delete!",
			val.chars, val.bytes);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

/*
 * Check that selection is replaced when we start typing.
 */
static int tbox_sel_keys_ascii(void)
{
	gp_widget *tbox;
	gp_utf8_pos val;
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
	if (val.chars != 7 || val.bytes != 7) {
		tst_msg("Wrong cursor position %zu %zu after text input!",
			val.chars, val.bytes);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int sel_whole_left(gp_widget *tbox)
{
	gp_widget_tbox_cursor_set(tbox, 0, GP_SEEK_END);
	state_press(GP_KEY_LEFT_SHIFT);
	send_keypress(tbox, GP_KEY_HOME);

	if (!gp_widget_tbox_sel(tbox)) {
		tst_msg("Text not selected after KEY_HOME + SHIFT");
		return TST_FAILED;
	}

	state_release(GP_KEY_LEFT_SHIFT);

	tst_msg("Selecting whole buffer from left");

	return TST_SUCCESS;
}

static int sel_whole_right(gp_widget *tbox)
{
	gp_widget_tbox_cursor_set(tbox, 0, GP_SEEK_SET);
	state_press(GP_KEY_LEFT_SHIFT);
	send_keypress(tbox, GP_KEY_END);

	if (!gp_widget_tbox_sel(tbox)) {
		tst_msg("Text not selected after KEY_END + SHIFT");
		return TST_FAILED;
	}

	state_release(GP_KEY_LEFT_SHIFT);

	tst_msg("Selecting whole buffer from right");

	return TST_SUCCESS;
}

static int check_cleared(gp_widget *tbox, const char *op, size_t cur_pos)
{
	gp_utf8_pos val;

	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Selection not cleared by %s!", op);
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val.chars != cur_pos) {
		tst_msg("Wrong cursor %zu ater %s expected %zu!",
		        val.chars, op, cur_pos);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int check_selected(gp_widget *tbox, const char *op, size_t sel_off, size_t sel_len)
{
	gp_utf8_pos val;

	val = gp_widget_tbox_sel_off(tbox);
	if (val.chars != sel_off) {
		tst_msg("Wrong selection after %s offset %zu expected %zu",
		        op, val.chars, sel_off);
		return TST_FAILED;
	}

	val = gp_widget_tbox_sel_len(tbox);
	if (val.chars != sel_len) {
		tst_msg("Wrong selection after %s lenght %zu expected %zu",
		        op, val.chars, sel_len);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

/*
 * Check that cursor movement clears selection.
 */
static int tbox_sel_keys_clear(void)
{
	gp_widget *tbox;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, 1, GP_SEEK_SET, 1);
	send_keypress(tbox, GP_KEY_LEFT);
	if (check_cleared(tbox, "KEY_LEFT", 1))
		return TST_FAILED;

	gp_widget_tbox_sel_set(tbox, 1, GP_SEEK_SET, 1);
	send_keypress(tbox, GP_KEY_RIGHT);
	if (check_cleared(tbox, "KEY_RIGHT", 2))
		return TST_FAILED;

	gp_widget_tbox_sel_set(tbox, 1, GP_SEEK_SET, 1);
	send_keypress(tbox, GP_KEY_HOME);
	if (check_cleared(tbox, "KEY_HOME", 0))
		return TST_FAILED;

	gp_widget_tbox_sel_set(tbox, 1, GP_SEEK_SET, 1);
	send_keypress(tbox, GP_KEY_END);
	if (check_cleared(tbox, "KEY_END", 11))
		return TST_FAILED;

	gp_widget_tbox_sel_all(tbox);
	gp_widget_tbox_cursor_set(tbox, 5, GP_SEEK_SET);
	if (check_cleared(tbox, "gp_widget_tbox_cursor_set()", 5))
		return TST_FAILED;

	return TST_SUCCESS;
}

/*
 * Check that functions that modify the buffer clears the selection.
 */
static int tbox_sel_change_clear(void)
{
	gp_widget *tbox;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_all(tbox);
	gp_widget_tbox_printf(tbox, "new text");
	if (check_cleared(tbox, "gp_widget_tbox_printf()", 8))
		return TST_FAILED;

	gp_widget_tbox_sel_all(tbox);
	gp_widget_tbox_ins(tbox, 3, GP_SEEK_SET, "ins");
	if (check_cleared(tbox, "gp_widget_tbox_ins()", 11))
		return TST_FAILED;

	gp_widget_tbox_sel_all(tbox);
	gp_widget_tbox_del(tbox, 3, GP_SEEK_SET, 3);
	if (check_cleared(tbox, "gp_widget_tbox_del()", 8))
		return TST_FAILED;

	gp_widget_tbox_sel_all(tbox);
	gp_widget_tbox_clear(tbox);
	if (check_cleared(tbox, "gp_widget_tbox_clear()", 0))
		return TST_FAILED;

	return TST_SUCCESS;
}

/*
 * Check that selection is cleared as well when a key to move cursor is pressed
 * but cursor cannot move. In this case the whole buffer is selected and as the
 * cursor can be either at left side of the selection or at a right side of it
 * it cannot move in half of the cases.
 */
static int tbox_sel_keys_clear_end(void)
{
	gp_widget *tbox;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	/* Select from left attempt to move left */
	if (sel_whole_left(tbox))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_LEFT);
	if (check_cleared(tbox, "KEY_LEFT", 0))
		return TST_FAILED;

	if (sel_whole_left(tbox))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_HOME);
	if (check_cleared(tbox, "KEY_HOME", 0))
		return TST_FAILED;

	/* Select from left attempt to move right */
	if (sel_whole_left(tbox))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_RIGHT);
	if (check_cleared(tbox, "KEY_RIGHT", 11))
		return TST_FAILED;

	if (sel_whole_left(tbox))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_END);
	if (check_cleared(tbox, "KEY_END", 11))
		return TST_FAILED;

	/* Select from right attempt to move left */
	if (sel_whole_right(tbox))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_LEFT);
	if (check_cleared(tbox, "KEY_LEFT", 0))
		return TST_FAILED;

	if (sel_whole_right(tbox))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_HOME);
	if (check_cleared(tbox, "KEY_HOME", 0))
		return TST_FAILED;

	/* Select from right attempt to move right */
	if (sel_whole_right(tbox))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_RIGHT);
	if (check_cleared(tbox, "KEY_RIGHT", 11))
		return TST_FAILED;

	if (sel_whole_right(tbox))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_END);
	if (check_cleared(tbox, "KEY_END", 11))
		return TST_FAILED;

	return TST_SUCCESS;
}

/*
 * Check that backspace and delete removed selected text.
 */
static int tbox_sel_key_del_backspace(int key)
{
	gp_widget *tbox;
	const char *str;
	gp_utf8_pos val;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, 2, GP_SEEK_SET, 7);
	send_keypress(tbox, key);

	str = gp_widget_tbox_text(tbox);
	if (strcmp(str, "held")) {
		tst_msg("Wrong text '%s' after deleted selection!", str);
		return TST_FAILED;
	}

	val = gp_widget_tbox_cursor_get(tbox);
	if (val.chars != 2 || val.bytes != 2) {
		tst_msg("Wrong cursor position %zu %zu after deleted selection!",
		        val.chars, val.bytes);
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

/*
 * Check that sequence of SHIFT + LEFT, SHIFT + RIGHT and the other way around
 * actually clears the selection.
 */
static int tbox_sel_key_forth_back(void)
{
	gp_widget *tbox;
	size_t cursor = 5;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, cursor, GP_SEEK_SET);
	state_press(GP_KEY_LEFT_SHIFT);

	send_keypress(tbox, GP_KEY_LEFT);
	if (check_selected(tbox, "KEY_LEFT", cursor - 1, 1))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_RIGHT);
	if (check_cleared(tbox, "KEY_RIGHT", cursor))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_RIGHT);
	if (check_selected(tbox, "KEY_RIGHT", cursor, 1))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_LEFT);
	if (check_cleared(tbox, "KEY_LEFT", cursor))
		return TST_FAILED;

	return TST_SUCCESS;
}

/*
 * Checks that comibination of shift+left and shift+end selects from selection
 * starting point not from cursor.
 */
static int tbox_sel_key_left_end(void)
{
	gp_widget *tbox;
	size_t cursor = 5;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, cursor, GP_SEEK_SET);
	state_press(GP_KEY_LEFT_SHIFT);

	send_keypress(tbox, GP_KEY_LEFT);
	if (check_selected(tbox, "KEY_LEFT", cursor - 1, 1))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_END);
	if (check_selected(tbox, "KEY_END", cursor, 6))
		return TST_FAILED;

	return TST_SUCCESS;
}

/*
 * Checks that comibination of shift+left and shift+home extends selection.
 */
static int tbox_sel_key_left_home(void)
{
	gp_widget *tbox;
	size_t cursor = 5;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, cursor, GP_SEEK_SET);
	state_press(GP_KEY_LEFT_SHIFT);

	send_keypress(tbox, GP_KEY_LEFT);
	if (check_selected(tbox, "KEY_LEFT", cursor - 1, 1))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_HOME);
	if (check_selected(tbox, "KEY_HOME", 0, cursor))
		return TST_FAILED;

	return TST_SUCCESS;
}

/*
 * Checks that comibination of shift+right and shift+home selects from selection
 * starting point not from cursor.
 */
static int tbox_sel_key_right_home(void)
{
	gp_widget *tbox;
	size_t cursor = 5;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, cursor, GP_SEEK_SET);
	state_press(GP_KEY_LEFT_SHIFT);

	send_keypress(tbox, GP_KEY_RIGHT);
	if (check_selected(tbox, "KEY_RIGHT", cursor, 1))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_HOME);
	if (check_selected(tbox, "KEY_HOME", 0, 5))
		return TST_FAILED;

	return TST_SUCCESS;
}

/*
 * Checks that comibination of shift+right and shift+end extends selection.
 */
static int tbox_sel_key_right_end(void)
{
	gp_widget *tbox;
	size_t cursor = 5;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, cursor, GP_SEEK_SET);
	state_press(GP_KEY_LEFT_SHIFT);

	send_keypress(tbox, GP_KEY_RIGHT);
	if (check_selected(tbox, "KEY_RIGHT", cursor, 1))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_END);
	if (check_selected(tbox, "KEY_END", cursor, 6))
		return TST_FAILED;

	return TST_SUCCESS;
}

/*
 * Check that Ctrl+a select whole buffer and esc clears the selection.
 */
static int tbox_sel_ctrl_a_esc(void)
{
	gp_widget *tbox;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL, 0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	state_press(GP_KEY_LEFT_CTRL);
	send_keypress(tbox, GP_KEY_A);
	state_release(GP_KEY_LEFT_CTRL);

	if (check_selected(tbox, "KEY_CTRL + KEY_A", 0, 11))
		return TST_FAILED;

	send_keypress(tbox, GP_KEY_ESC);

	if (check_cleared(tbox, "KEY_ESC", 11))
		return TST_FAILED;

	return TST_SUCCESS;
}

/*
 * Check that when hidden text flag is on text cannot be selected.
 */
static int tbox_hidden_no_sel(void)
{
	gp_widget *tbox;

	tbox = gp_widget_tbox_new("hello world", 0, 10, 0, NULL,
	                          GP_WIDGET_TBOX_HIDDEN, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_cursor_set(tbox, 2, GP_SEEK_SET);

	state_press(GP_KEY_LEFT_SHIFT);

	send_keypress(tbox, GP_KEY_LEFT);
	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Hidden text was selected with KEY_LEFT");
		return TST_FAILED;
	}

	send_keypress(tbox, GP_KEY_RIGHT);
	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Hidden text was selected with KEY_RIGHT");
		return TST_FAILED;
	}

	send_keypress(tbox, GP_KEY_HOME);
	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Hidden text was selected with KEY_HOME");
		return TST_FAILED;
	}

	send_keypress(tbox, GP_KEY_END);
	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Hidden text was selected with KEY_END");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_all(tbox);
	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Hidden text was selected with sel_all()");
		return TST_FAILED;
	}

	gp_widget_tbox_sel_set(tbox, 0, GP_SEEK_SET, 2);
	if (gp_widget_tbox_sel(tbox)) {
		tst_msg("Hidden text was selected with sel_set()");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tbox_utf_del(void)
{
	gp_widget *tbox;

	tbox = gp_widget_tbox_new("hello \u00d7", 0, 10, 0, NULL,
	                          0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tbox_del(tbox, -1, GP_SEEK_CUR, 1);

	if (strcmp(gp_widget_tbox_text(tbox), "hello ")) {
		tst_msg("Wrong tbox text after deleting utf8 char '%s'",
		        gp_widget_tbox_text(tbox));
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tbox_utf_del_key(void)
{
	gp_widget *tbox;

	tbox = gp_widget_tbox_new("hello \u00d7!", 0, 10, 0, NULL,
	                          0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	send_keypress(tbox, GP_KEY_LEFT);
	send_keypress(tbox, GP_KEY_LEFT);
	send_keypress(tbox, GP_KEY_DELETE);

	if (strcmp(gp_widget_tbox_text(tbox), "hello !")) {
		tst_msg("Wrong tbox text after deleting utf8 char '%s'",
		        gp_widget_tbox_text(tbox));
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tbox_utf_backspace_key(void)
{
	gp_widget *tbox;

	tbox = gp_widget_tbox_new("hello \u00d7!", 0, 10, 0, NULL,
	                          0, NULL, NULL);
	if (!tbox) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	send_keypress(tbox, GP_KEY_LEFT);
	send_keypress(tbox, GP_KEY_BACKSPACE);

	if (strcmp(gp_widget_tbox_text(tbox), "hello !")) {
		tst_msg("Wrong tbox text after deleting utf8 char '%s'",
		        gp_widget_tbox_text(tbox));
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

		{.name = "tbox sel change clear",
		 .tst_fn = tbox_sel_change_clear},

		{.name = "tbox sel keys clear end",
		 .tst_fn = tbox_sel_keys_clear_end},

		{.name = "tbox sel ctrl a + esc",
		 .tst_fn = tbox_sel_ctrl_a_esc},

		{.name = "tbox sel key del",
		 .tst_fn = tbox_sel_key_del},

		{.name = "tbox sel key backspace",
		 .tst_fn = tbox_sel_key_backspace},

		{.name = "tbox sel key forth back",
		 .tst_fn = tbox_sel_key_forth_back},

		{.name = "tbox sel key left end",
		 .tst_fn = tbox_sel_key_left_end},

		{.name = "tbox sel key left home",
		 .tst_fn = tbox_sel_key_left_home},

		{.name = "tbox sel key right home",
		 .tst_fn = tbox_sel_key_right_home},

		{.name = "tbox sel key right end",
		 .tst_fn = tbox_sel_key_right_end},

		{.name = "tbox hidden no sel",
		 .tst_fn = tbox_hidden_no_sel},

		{.name = "tbox utf del",
		 .tst_fn = tbox_utf_del},

		{.name = "tbox utf del key",
		 .tst_fn = tbox_utf_del_key},

		{.name = "tbox utf backspace key",
		 .tst_fn = tbox_utf_backspace_key},

		{.name = NULL},
	}
};
