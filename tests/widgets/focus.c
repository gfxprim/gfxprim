// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Tests that the widget::focused flag is set exactly on the path from the
 * layout root to the focused leaf regardless of how the focus was moved.
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

#define LOG_SIZE 16

static struct focus_log {
	gp_widget *widget;
	int sub_type;
} focus_log[LOG_SIZE];

static unsigned int focus_log_cnt;

static int log_focus(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_FOCUS)
		return 0;

	if (focus_log_cnt >= LOG_SIZE)
		return 0;

	focus_log[focus_log_cnt].widget = ev->self;
	focus_log[focus_log_cnt].sub_type = ev->sub_type;
	focus_log_cnt++;

	return 0;
}

static void log_widget(gp_widget *self)
{
	gp_widget_on_event_set(self, log_focus, NULL);
	gp_widget_events_unmask(self, GP_WIDGET_EVENT_FOCUS);
}

static int log_find(gp_widget *self, int sub_type)
{
	unsigned int i;

	for (i = 0; i < focus_log_cnt; i++) {
		if (focus_log[i].widget == self && focus_log[i].sub_type == sub_type)
			return i;
	}

	return -1;
}

struct expect {
	const char *name;
	gp_widget *widget;
	int focused;
};

static int check_focused(const char *when, struct expect *exp)
{
	int ret = 0;

	for (; exp->name; exp++) {
		if (!!exp->widget->focused == exp->focused)
			continue;

		tst_msg("%s: %s focused=%i expected %i", when, exp->name,
		        exp->widget->focused, exp->focused);
		ret = 1;
	}

	return ret;
}

/*
 * G = grid[ A | B ], A = grid[a1, a2], B = grid[b1]
 */
static int focus_set_after_keyboard(void)
{
	gp_widget *G, *A, *B, *a1, *a2, *b1;
	int a1_in, b1_out;

	G = gp_widget_grid_new(2, 1, 0);
	A = gp_widget_grid_new(1, 2, 0);
	B = gp_widget_grid_new(1, 1, 0);
	a1 = gp_widget_button_new("a1", GP_BUTTON_LABEL);
	a2 = gp_widget_button_new("a2", GP_BUTTON_LABEL);
	b1 = gp_widget_button_new("b1", GP_BUTTON_LABEL);

	if (!G || !A || !B || !a1 || !a2 || !b1) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, A);
	gp_widget_grid_put(G, 1, 0, B);
	gp_widget_grid_put(A, 0, 0, a1);
	gp_widget_grid_put(A, 0, 1, a2);
	gp_widget_grid_put(B, 0, 0, b1);

	log_widget(a1);
	log_widget(b1);

	struct expect on_a1[] = {
		{"G", G, 1}, {"A", A, 1}, {"a1", a1, 1}, {"a2", a2, 0},
		{"B", B, 0}, {"b1", b1, 0}, {}
	};

	struct expect on_b1[] = {
		{"G", G, 1}, {"A", A, 0}, {"a1", a1, 0}, {"a2", a2, 0},
		{"B", B, 1}, {"b1", b1, 1}, {}
	};

	if (!gp_widget_focus_set(a1)) {
		tst_msg("Failed to focus a1");
		goto fail;
	}

	if (check_focused("focus_set(a1)", on_a1))
		goto fail;

	/* a1 -> a2 -> b1 */
	gp_widget_ops_render_focus(G, GP_FOCUS_NEXT);
	gp_widget_ops_render_focus(G, GP_FOCUS_NEXT);

	if (check_focused("focus next", on_b1))
		goto fail;

	focus_log_cnt = 0;

	if (!gp_widget_focus_set(a1)) {
		tst_msg("Failed to focus a1");
		goto fail;
	}

	if (check_focused("focus_set(a1) after focus next", on_a1))
		goto fail;

	a1_in = log_find(a1, GP_WIDGET_EVENT_FOCUS_IN);
	b1_out = log_find(b1, GP_WIDGET_EVENT_FOCUS_OUT);

	if (a1_in < 0 || b1_out < 0) {
		tst_msg("Missing focus events a1 in %i b1 out %i", a1_in, b1_out);
		goto fail;
	}

	if (b1_out > a1_in) {
		tst_msg("b1 focus out sent after a1 focus in");
		goto fail;
	}

	/* Focusing already focused widget is no-op */
	focus_log_cnt = 0;

	if (!gp_widget_focus_set(a1) || focus_log_cnt) {
		tst_msg("Focusing focused widget failed or generated events");
		goto fail;
	}

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * G = grid[ S | b ], S = scroll_area(a)
 */
static int focus_set_scroll_area(void)
{
	gp_widget *G, *S, *a, *b;

	G = gp_widget_grid_new(2, 1, 0);
	a = gp_widget_button_new("a", GP_BUTTON_LABEL);
	b = gp_widget_button_new("b", GP_BUTTON_LABEL);
	S = gp_widget_scroll_area_new(10, 10, a);

	if (!G || !S || !a || !b) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, S);
	gp_widget_grid_put(G, 1, 0, b);

	struct expect on_a[] = {
		{"G", G, 1}, {"S", S, 1}, {"a", a, 1}, {"b", b, 0}, {}
	};

	struct expect on_b[] = {
		{"G", G, 1}, {"S", S, 0}, {"a", a, 0}, {"b", b, 1}, {}
	};

	if (!gp_widget_focus_set(a)) {
		tst_msg("Failed to focus a");
		goto fail;
	}

	if (check_focused("focus_set(a)", on_a))
		goto fail;

	if (!gp_widget_focus_set(b)) {
		tst_msg("Failed to focus b");
		goto fail;
	}

	if (check_focused("focus_set(b)", on_b))
		goto fail;

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * G = grid[ L | b ], L = layout_switch(a1, a2), a1 is the active layout
 */
static int focus_set_fail(void)
{
	gp_widget *G, *L, *a1, *a2, *b;

	G = gp_widget_grid_new(2, 1, 0);
	L = gp_widget_layout_switch_new(2);
	a1 = gp_widget_button_new("a1", GP_BUTTON_LABEL);
	a2 = gp_widget_button_new("a2", GP_BUTTON_LABEL);
	b = gp_widget_button_new("b", GP_BUTTON_LABEL);

	if (!G || !L || !a1 || !a2 || !b) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, L);
	gp_widget_grid_put(G, 1, 0, b);
	gp_widget_layout_switch_put(L, 0, a1);
	gp_widget_layout_switch_put(L, 1, a2);

	log_widget(a2);
	log_widget(b);

	struct expect on_b[] = {
		{"G", G, 1}, {"L", L, 0}, {"a1", a1, 0}, {"a2", a2, 0},
		{"b", b, 1}, {}
	};

	if (!gp_widget_focus_set(b)) {
		tst_msg("Failed to focus b");
		goto fail;
	}

	focus_log_cnt = 0;

	if (gp_widget_focus_set(a2)) {
		tst_msg("Focused widget in inactive layout");
		goto fail;
	}

	if (check_focused("failed focus_set(a2)", on_b))
		goto fail;

	if (focus_log_cnt) {
		tst_msg("Failed focus_set() generated %u events", focus_log_cnt);
		goto fail;
	}

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * G = grid[ A | B ], A = grid[a1, a2], B = grid[b1]
 */
static int focus_keyboard(void)
{
	gp_widget *G, *A, *B, *a1, *a2, *b1;

	G = gp_widget_grid_new(2, 1, 0);
	A = gp_widget_grid_new(1, 2, 0);
	B = gp_widget_grid_new(1, 1, 0);
	a1 = gp_widget_button_new("a1", GP_BUTTON_LABEL);
	a2 = gp_widget_button_new("a2", GP_BUTTON_LABEL);
	b1 = gp_widget_button_new("b1", GP_BUTTON_LABEL);

	if (!G || !A || !B || !a1 || !a2 || !b1) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, A);
	gp_widget_grid_put(G, 1, 0, B);
	gp_widget_grid_put(A, 0, 0, a1);
	gp_widget_grid_put(A, 0, 1, a2);
	gp_widget_grid_put(B, 0, 0, b1);

	struct expect none[] = {
		{"G", G, 0}, {"A", A, 0}, {"a1", a1, 0}, {"a2", a2, 0},
		{"B", B, 0}, {"b1", b1, 0}, {}
	};

	struct expect on_a1[] = {
		{"G", G, 1}, {"A", A, 1}, {"a1", a1, 1}, {"a2", a2, 0},
		{"B", B, 0}, {"b1", b1, 0}, {}
	};

	struct expect on_a2[] = {
		{"G", G, 1}, {"A", A, 1}, {"a1", a1, 0}, {"a2", a2, 1},
		{"B", B, 0}, {"b1", b1, 0}, {}
	};

	struct expect on_b1[] = {
		{"G", G, 1}, {"A", A, 0}, {"a1", a1, 0}, {"a2", a2, 0},
		{"B", B, 1}, {"b1", b1, 1}, {}
	};

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next into layout", on_a1))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next", on_a2))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next", on_b1))
		goto fail;

	if (gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next on last widget", on_b1))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_PREV) ||
	    check_focused("focus prev", on_a2))
		goto fail;

	/* There is nothing left of a2 */
	if (gp_widget_ops_render_focus(G, GP_FOCUS_LEFT) ||
	    check_focused("focus left", on_a2))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_RIGHT) ||
	    check_focused("focus right", on_b1))
		goto fail;

	if (!gp_widget_ops_focus_out(G) ||
	    check_focused("focus out", none))
		goto fail;

	/* Focus moves back where it was */
	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next after focus out", on_b1))
		goto fail;

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * G = grid[ T | b ], T = tabs(a)
 */
static int focus_tabs(void)
{
	const char *labels[] = {"tab"};
	gp_widget *G, *T, *a, *b;

	G = gp_widget_grid_new(2, 1, 0);
	T = gp_widget_tabs_new(1, 0, labels, 0);
	a = gp_widget_button_new("a", GP_BUTTON_LABEL);
	b = gp_widget_button_new("b", GP_BUTTON_LABEL);

	if (!G || !T || !a || !b) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, T);
	gp_widget_grid_put(G, 1, 0, b);
	gp_widget_tabs_put(T, 0, a);

	/* Title is focused when T is focused and a isn't */
	struct expect on_title[] = {
		{"G", G, 1}, {"T", T, 1}, {"a", a, 0}, {"b", b, 0}, {}
	};

	struct expect on_a[] = {
		{"G", G, 1}, {"T", T, 1}, {"a", a, 1}, {"b", b, 0}, {}
	};

	struct expect on_b[] = {
		{"G", G, 1}, {"T", T, 0}, {"a", a, 0}, {"b", b, 1}, {}
	};

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next into layout", on_title))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next", on_a))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next", on_b))
		goto fail;

	/* Moving backwards into tabs focuses the widget first */
	if (!gp_widget_ops_render_focus(G, GP_FOCUS_PREV) ||
	    check_focused("focus prev", on_a))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_PREV) ||
	    check_focused("focus prev", on_title))
		goto fail;

	if (!gp_widget_focus_set(a) ||
	    check_focused("focus_set(a)", on_a))
		goto fail;

	if (!gp_widget_focus_set(T) ||
	    check_focused("focus_set(T)", on_title))
		goto fail;

	if (!gp_widget_focus_set(b) ||
	    check_focused("focus_set(b)", on_b))
		goto fail;

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * G = grid[ T | c ], T = tabs(A, B, l), A = grid[a1, a2], B = grid[b1, b2]
 *
 * The l is a label that cannot be focused.
 */
static int focus_tabs_switch(void)
{
	const char *labels[] = {"A", "B", "l"};
	gp_widget *G, *T, *A, *B, *a1, *a2, *b1, *b2, *c, *l;

	G = gp_widget_grid_new(2, 1, 0);
	T = gp_widget_tabs_new(3, 0, labels, 0);
	A = gp_widget_grid_new(1, 2, 0);
	B = gp_widget_grid_new(1, 2, 0);
	a1 = gp_widget_button_new("a1", GP_BUTTON_LABEL);
	a2 = gp_widget_button_new("a2", GP_BUTTON_LABEL);
	b1 = gp_widget_button_new("b1", GP_BUTTON_LABEL);
	b2 = gp_widget_button_new("b2", GP_BUTTON_LABEL);
	c = gp_widget_button_new("c", GP_BUTTON_LABEL);
	l = gp_widget_label_new("l", 0, 0);

	if (!G || !T || !A || !B || !a1 || !a2 || !b1 || !b2 || !c || !l) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, T);
	gp_widget_grid_put(G, 1, 0, c);
	gp_widget_tabs_put(T, 0, A);
	gp_widget_tabs_put(T, 1, B);
	gp_widget_tabs_put(T, 2, l);
	gp_widget_grid_put(A, 0, 0, a1);
	gp_widget_grid_put(A, 0, 1, a2);
	gp_widget_grid_put(B, 0, 0, b1);
	gp_widget_grid_put(B, 0, 1, b2);

	struct expect on_a2[] = {
		{"T", T, 1}, {"A", A, 1}, {"a2", a2, 1},
		{"B", B, 0}, {"b1", b1, 0}, {"b2", b2, 0}, {"c", c, 0}, {}
	};

	struct expect on_b1[] = {
		{"T", T, 1}, {"A", A, 0}, {"a2", a2, 0},
		{"B", B, 1}, {"b1", b1, 1}, {"b2", b2, 0}, {"c", c, 0}, {}
	};

	struct expect on_b2[] = {
		{"T", T, 1}, {"A", A, 0}, {"a2", a2, 0},
		{"B", B, 1}, {"b1", b1, 0}, {"b2", b2, 1}, {"c", c, 0}, {}
	};

	/* Title is focused when T is focused and nothing in it is */
	struct expect on_title[] = {
		{"T", T, 1}, {"A", A, 0}, {"a2", a2, 0},
		{"B", B, 0}, {"b1", b1, 0}, {"b2", b2, 0}, {"c", c, 0}, {}
	};

	if (!gp_widget_focus_set(a2) ||
	    check_focused("focus_set(a2)", on_a2))
		goto fail;

	gp_widget_tabs_active_set(T, 1);
	if (check_focused("switch to tab B", on_b1))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next", on_b2))
		goto fail;

	/* Focus is remembered per tab */
	gp_widget_tabs_active_set(T, 0);
	if (check_focused("switch back to tab A", on_a2))
		goto fail;

	gp_widget_tabs_active_set(T, 1);
	if (check_focused("switch back to tab B", on_b2))
		goto fail;

	/* Nothing to focus in the tab, falls back to the title */
	gp_widget_tabs_active_set(T, 2);
	if (check_focused("switch to tab l", on_title))
		goto fail;

	/* Switching tabs with the title focused keeps the title focused */
	gp_widget_tabs_active_set(T, 0);
	if (check_focused("switch to tab A from title", on_title))
		goto fail;

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * G = grid[ L | c ], L = layout_switch(a, l)
 *
 * The l is a label that cannot be focused.
 */
static int focus_layout_switch(void)
{
	gp_widget *G, *L, *a, *c, *l;

	G = gp_widget_grid_new(2, 1, 0);
	L = gp_widget_layout_switch_new(2);
	a = gp_widget_button_new("a", GP_BUTTON_LABEL);
	c = gp_widget_button_new("c", GP_BUTTON_LABEL);
	l = gp_widget_label_new("l", 0, 0);

	if (!G || !L || !a || !c || !l) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, L);
	gp_widget_grid_put(G, 1, 0, c);
	gp_widget_layout_switch_put(L, 0, a);
	gp_widget_layout_switch_put(L, 1, l);

	struct expect on_a[] = {
		{"G", G, 1}, {"L", L, 1}, {"a", a, 1}, {"c", c, 0}, {}
	};

	struct expect on_c[] = {
		{"G", G, 1}, {"L", L, 0}, {"a", a, 0}, {"c", c, 1}, {}
	};

	if (!gp_widget_focus_set(a) ||
	    check_focused("focus_set(a)", on_a))
		goto fail;

	/* Nothing to focus in L, focus moves to the next widget */
	gp_widget_layout_switch_layout(L, 1);
	if (check_focused("switch to layout l", on_c))
		goto fail;

	/* L is not focused, nothing changes */
	gp_widget_layout_switch_layout(L, 0);
	if (check_focused("switch to layout a", on_c))
		goto fail;

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * G = grid[a, b, c] in a column
 */
static int focus_grid_remove(void)
{
	gp_widget *G, *a, *b, *c;

	G = gp_widget_grid_new(1, 3, 0);
	a = gp_widget_button_new("a", GP_BUTTON_LABEL);
	b = gp_widget_button_new("b", GP_BUTTON_LABEL);
	c = gp_widget_button_new("c", GP_BUTTON_LABEL);

	if (!G || !a || !b || !c) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, a);
	gp_widget_grid_put(G, 0, 1, b);
	gp_widget_grid_put(G, 0, 2, c);

	struct expect on_a[] = {
		{"G", G, 1}, {"a", a, 1}, {"c", c, 0}, {}
	};

	struct expect on_c[] = {
		{"G", G, 1}, {"a", a, 0}, {"c", c, 1}, {}
	};

	if (!gp_widget_focus_set(b)) {
		tst_msg("Failed to focus b");
		goto fail;
	}

	/* Focus moves to the first widget */
	gp_widget_grid_rem(G, 0, 1);

	if (b->focused) {
		tst_msg("Removed widget b is still focused");
		gp_widget_free(b);
		goto fail;
	}

	gp_widget_free(b);

	if (check_focused("grid rem focused b", on_a))
		goto fail;

	/* Focused cell has to move with the row insert */
	if (!gp_widget_focus_set(c))
		goto fail;

	gp_widget_grid_rows_ins(G, 0, 1);
	gp_widget_ops_focus_out(G);

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("rows ins, focus out and focus next", on_c))
		goto fail;

	/* Deletes c, focus moves to the first widget */
	gp_widget_grid_rows_del(G, 3, 1);

	struct expect on_a_only[] = {
		{"G", G, 1}, {"a", a, 1}, {}
	};

	if (check_focused("rows del focused c", on_a_only))
		goto fail;

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * O = overlay(a, b), b is on the top
 */
static int focus_overlay_hide(void)
{
	gp_widget *O, *a, *b;

	O = gp_widget_overlay_new(2);
	a = gp_widget_button_new("a", GP_BUTTON_LABEL);
	b = gp_widget_button_new("b", GP_BUTTON_LABEL);

	if (!O || !a || !b) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_overlay_put(O, 0, a);
	gp_widget_overlay_put(O, 1, b);

	struct expect on_a[] = {
		{"O", O, 1}, {"a", a, 1}, {"b", b, 0}, {}
	};

	struct expect on_b[] = {
		{"O", O, 1}, {"a", a, 0}, {"b", b, 1}, {}
	};

	if (!gp_widget_ops_render_focus(O, GP_FOCUS_NEXT) ||
	    check_focused("focus next into overlay", on_b))
		goto fail;

	gp_widget_overlay_hide(O, 1);
	if (check_focused("hide focused layer", on_a))
		goto fail;

	gp_widget_free(O);
	return TST_PASSED;
fail:
	gp_widget_free(O);
	return TST_FAILED;
}

struct expect_ev {
	const char *name;
	gp_widget *widget;
	int sub_type;
};

static const char *ev_name(int sub_type)
{
	return sub_type == GP_WIDGET_EVENT_FOCUS_IN ? "in" : "out";
}

static int check_log(const char *when, struct expect_ev *exp)
{
	unsigned int i;

	for (i = 0; exp[i].name; i++) {
		if (i >= focus_log_cnt) {
			tst_msg("%s: missing event %u %s %s", when, i,
			        exp[i].name, ev_name(exp[i].sub_type));
			return 1;
		}

		if (focus_log[i].widget != exp[i].widget ||
		    focus_log[i].sub_type != exp[i].sub_type) {
			tst_msg("%s: event %u expected %s %s", when, i,
			        exp[i].name, ev_name(exp[i].sub_type));
			return 1;
		}
	}

	if (i != focus_log_cnt) {
		tst_msg("%s: %u unexpected events", when, focus_log_cnt - i);
		return 1;
	}

	return 0;
}

/*
 * G = grid[ A | B ], A = grid[A1], A1 = grid[a1, a2], B = grid[B1], B1 = grid[b1]
 */
static int focus_nested(void)
{
	gp_widget *G, *A, *A1, *B, *B1, *a1, *a2, *b1;

	G = gp_widget_grid_new(2, 1, 0);
	A = gp_widget_grid_new(1, 1, 0);
	A1 = gp_widget_grid_new(1, 2, 0);
	B = gp_widget_grid_new(1, 1, 0);
	B1 = gp_widget_grid_new(1, 1, 0);
	a1 = gp_widget_button_new("a1", GP_BUTTON_LABEL);
	a2 = gp_widget_button_new("a2", GP_BUTTON_LABEL);
	b1 = gp_widget_button_new("b1", GP_BUTTON_LABEL);

	if (!G || !A || !A1 || !B || !B1 || !a1 || !a2 || !b1) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, A);
	gp_widget_grid_put(G, 1, 0, B);
	gp_widget_grid_put(A, 0, 0, A1);
	gp_widget_grid_put(A1, 0, 0, a1);
	gp_widget_grid_put(A1, 0, 1, a2);
	gp_widget_grid_put(B, 0, 0, B1);
	gp_widget_grid_put(B1, 0, 0, b1);

	log_widget(G);
	log_widget(A);
	log_widget(A1);
	log_widget(B);
	log_widget(B1);
	log_widget(a1);
	log_widget(a2);
	log_widget(b1);

	struct expect on_a1[] = {
		{"G", G, 1}, {"A", A, 1}, {"A1", A1, 1}, {"a1", a1, 1},
		{"a2", a2, 0}, {"B", B, 0}, {"B1", B1, 0}, {"b1", b1, 0}, {}
	};

	struct expect on_a2[] = {
		{"G", G, 1}, {"A", A, 1}, {"A1", A1, 1}, {"a1", a1, 0},
		{"a2", a2, 1}, {"B", B, 0}, {"B1", B1, 0}, {"b1", b1, 0}, {}
	};

	struct expect on_b1[] = {
		{"G", G, 1}, {"A", A, 0}, {"A1", A1, 0}, {"a1", a1, 0},
		{"a2", a2, 0}, {"B", B, 1}, {"B1", B1, 1}, {"b1", b1, 1}, {}
	};

	struct expect_ev ev_focus_a1[] = {
		{"G", G, GP_WIDGET_EVENT_FOCUS_IN},
		{"A", A, GP_WIDGET_EVENT_FOCUS_IN},
		{"A1", A1, GP_WIDGET_EVENT_FOCUS_IN},
		{"a1", a1, GP_WIDGET_EVENT_FOCUS_IN},
		{}
	};

	/* Old path is unfocused bottom-up up to G, new focused top-down */
	struct expect_ev ev_a1_to_b1[] = {
		{"a1", a1, GP_WIDGET_EVENT_FOCUS_OUT},
		{"A1", A1, GP_WIDGET_EVENT_FOCUS_OUT},
		{"A", A, GP_WIDGET_EVENT_FOCUS_OUT},
		{"B", B, GP_WIDGET_EVENT_FOCUS_IN},
		{"B1", B1, GP_WIDGET_EVENT_FOCUS_IN},
		{"b1", b1, GP_WIDGET_EVENT_FOCUS_IN},
		{}
	};

	/* Shift+Tab enters A at its last widget */
	struct expect_ev ev_b1_to_a2[] = {
		{"b1", b1, GP_WIDGET_EVENT_FOCUS_OUT},
		{"B1", B1, GP_WIDGET_EVENT_FOCUS_OUT},
		{"B", B, GP_WIDGET_EVENT_FOCUS_OUT},
		{"A", A, GP_WIDGET_EVENT_FOCUS_IN},
		{"A1", A1, GP_WIDGET_EVENT_FOCUS_IN},
		{"a2", a2, GP_WIDGET_EVENT_FOCUS_IN},
		{}
	};

	/* Common ancestor is A1, nothing above it gets events */
	struct expect_ev ev_a2_to_a1[] = {
		{"a2", a2, GP_WIDGET_EVENT_FOCUS_OUT},
		{"a1", a1, GP_WIDGET_EVENT_FOCUS_IN},
		{}
	};

	struct expect_ev ev_focus_out[] = {
		{"a2", a2, GP_WIDGET_EVENT_FOCUS_OUT},
		{"A1", A1, GP_WIDGET_EVENT_FOCUS_OUT},
		{"A", A, GP_WIDGET_EVENT_FOCUS_OUT},
		{"G", G, GP_WIDGET_EVENT_FOCUS_OUT},
		{}
	};

	struct expect_ev ev_none[] = {
		{}
	};

	focus_log_cnt = 0;
	if (!gp_widget_focus_set(a1) ||
	    check_focused("focus_set(a1)", on_a1) ||
	    check_log("focus_set(a1)", ev_focus_a1))
		goto fail;

	focus_log_cnt = 0;
	if (!gp_widget_focus_set(b1) ||
	    check_focused("focus_set(b1)", on_b1) ||
	    check_log("focus_set(b1)", ev_a1_to_b1))
		goto fail;

	focus_log_cnt = 0;
	if (!gp_widget_ops_render_focus(G, GP_FOCUS_PREV) ||
	    check_focused("focus prev from b1", on_a2) ||
	    check_log("focus prev from b1", ev_b1_to_a2))
		goto fail;

	focus_log_cnt = 0;
	if (!gp_widget_ops_render_focus(G, GP_FOCUS_PREV) ||
	    check_focused("focus prev from a2", on_a1) ||
	    check_log("focus prev from a2", ev_a2_to_a1))
		goto fail;

	/* Tab enters B at its first widget */
	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    !gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next twice from a1", on_b1))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_PREV) ||
	    check_focused("focus prev from b1", on_a2))
		goto fail;

	focus_log_cnt = 0;
	if (!gp_widget_focus_set(a2) ||
	    check_log("focus_set(a2) on focused a2", ev_none))
		goto fail;

	focus_log_cnt = 0;
	if (!gp_widget_ops_focus_out(G) ||
	    check_log("focus out", ev_focus_out))
		goto fail;

	/* Focus moves back where it was, not to the first widget */
	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next after focus out", on_a2))
		goto fail;

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * G = grid[ b | F ], F = frame(L), L = layout_switch(a)
 *
 * Frame and layout switch have a single child and no focus_child_next().
 */
static int focus_single_child(void)
{
	gp_widget *G, *F, *L, *a, *b;

	G = gp_widget_grid_new(2, 1, 0);
	L = gp_widget_layout_switch_new(1);
	F = gp_widget_frame_new("frame", 0, L);
	a = gp_widget_button_new("a", GP_BUTTON_LABEL);
	b = gp_widget_button_new("b", GP_BUTTON_LABEL);

	if (!G || !F || !L || !a || !b) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, b);
	gp_widget_grid_put(G, 1, 0, F);
	gp_widget_layout_switch_put(L, 0, a);

	struct expect on_a[] = {
		{"G", G, 1}, {"F", F, 1}, {"L", L, 1}, {"a", a, 1}, {"b", b, 0}, {}
	};

	struct expect on_b[] = {
		{"G", G, 1}, {"F", F, 0}, {"L", L, 0}, {"a", a, 0}, {"b", b, 1}, {}
	};

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next into layout", on_b))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next into frame", on_a))
		goto fail;

	if (gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next on last widget", on_a))
		goto fail;

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_PREV) ||
	    check_focused("focus prev", on_b))
		goto fail;

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * G = grid[ b | O ], O = overlay(a, x), x is on the top
 */
static int focus_overlay_enter(void)
{
	gp_widget *G, *O, *a, *b, *x;

	G = gp_widget_grid_new(2, 1, 0);
	O = gp_widget_overlay_new(2);
	a = gp_widget_button_new("a", GP_BUTTON_LABEL);
	b = gp_widget_button_new("b", GP_BUTTON_LABEL);
	x = gp_widget_button_new("x", GP_BUTTON_LABEL);

	if (!G || !O || !a || !b || !x) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, b);
	gp_widget_grid_put(G, 1, 0, O);
	gp_widget_overlay_put(O, 0, a);
	gp_widget_overlay_put(O, 1, x);

	struct expect on_a[] = {
		{"G", G, 1}, {"O", O, 1}, {"a", a, 1}, {"b", b, 0}, {"x", x, 0}, {}
	};

	struct expect on_x[] = {
		{"G", G, 1}, {"O", O, 1}, {"a", a, 0}, {"b", b, 0}, {"x", x, 1}, {}
	};

	/* Focus does not move between layers while the overlay is focused */
	if (!gp_widget_focus_set(x) ||
	    check_focused("focus_set(x)", on_x))
		goto fail;

	if (gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next from x", on_x))
		goto fail;

	/*
	 * The overlay remembers x as focused, entering the overlay has to
	 * continue to the layer below when x cannot be focused.
	 */
	if (!gp_widget_focus_set(b))
		goto fail;

	gp_widget_disable(x);

	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("focus next into overlay", on_a))
		goto fail;

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

/*
 * G = grid[ A | b | c ], A = grid[a1, a2]
 */
static int focus_disable(void)
{
	gp_widget *G, *A, *a1, *a2, *b, *c;

	G = gp_widget_grid_new(3, 1, 0);
	A = gp_widget_grid_new(1, 2, 0);
	a1 = gp_widget_button_new("a1", GP_BUTTON_LABEL);
	a2 = gp_widget_button_new("a2", GP_BUTTON_LABEL);
	b = gp_widget_button_new("b", GP_BUTTON_LABEL);
	c = gp_widget_button_new("c", GP_BUTTON_LABEL);

	if (!G || !A || !a1 || !a2 || !b || !c) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_grid_put(G, 0, 0, A);
	gp_widget_grid_put(G, 1, 0, b);
	gp_widget_grid_put(G, 2, 0, c);
	gp_widget_grid_put(A, 0, 0, a1);
	gp_widget_grid_put(A, 0, 1, a2);

	struct expect none[] = {
		{"G", G, 0}, {"A", A, 0}, {"a1", a1, 0}, {"a2", a2, 0},
		{"b", b, 0}, {"c", c, 0}, {}
	};

	struct expect on_a1[] = {
		{"G", G, 1}, {"A", A, 1}, {"a1", a1, 1}, {"a2", a2, 0},
		{"b", b, 0}, {"c", c, 0}, {}
	};

	struct expect on_b[] = {
		{"G", G, 1}, {"A", A, 0}, {"a1", a1, 0}, {"a2", a2, 0},
		{"b", b, 1}, {"c", c, 0}, {}
	};

	struct expect on_c[] = {
		{"G", G, 1}, {"A", A, 0}, {"a1", a1, 0}, {"a2", a2, 0},
		{"b", b, 0}, {"c", c, 1}, {}
	};

	if (!gp_widget_focus_set(a2))
		goto fail;

	/* Disabling a container with the focused widget moves the focus next */
	gp_widget_disable(A);
	if (check_focused("disable A", on_b))
		goto fail;

	gp_widget_disable(b);
	if (check_focused("disable b", on_c))
		goto fail;

	/* Nothing left to focus */
	gp_widget_disable(c);
	if (check_focused("disable c", none))
		goto fail;

	/*
	 * Remembered c is disabled, focus moves into the layout at the first
	 * widget.
	 */
	gp_widget_enable(A);
	if (!gp_widget_ops_render_focus(G, GP_FOCUS_NEXT) ||
	    check_focused("enable A and focus next", on_a1))
		goto fail;

	/* There is nothing after c, the focus moves to the previous widget */
	gp_widget_enable(b);
	gp_widget_enable(c);

	if (!gp_widget_focus_set(c))
		goto fail;

	gp_widget_disable(c);
	if (check_focused("disable last widget c", on_b))
		goto fail;

	gp_widget_free(G);
	return TST_PASSED;
fail:
	gp_widget_free(G);
	return TST_FAILED;
}

const struct tst_suite tst_suite = {
	.suite_name = "widget focus testsuite",
	.tests = {
		{.name = "focus set after keyboard navigation",
		 .tst_fn = focus_set_after_keyboard,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus set scroll area",
		 .tst_fn = focus_set_scroll_area,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus set fail",
		 .tst_fn = focus_set_fail,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus keyboard",
		 .tst_fn = focus_keyboard,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus tabs",
		 .tst_fn = focus_tabs,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus tabs switch",
		 .tst_fn = focus_tabs_switch,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus layout switch",
		 .tst_fn = focus_layout_switch,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus grid remove",
		 .tst_fn = focus_grid_remove,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus overlay hide",
		 .tst_fn = focus_overlay_hide,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus nested events and order",
		 .tst_fn = focus_nested,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus single child containers",
		 .tst_fn = focus_single_child,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus overlay enter",
		 .tst_fn = focus_overlay_enter,
		 .flags = TST_CHECK_MALLOC},

		{.name = "focus disable",
		 .tst_fn = focus_disable,
		 .flags = TST_CHECK_MALLOC},

		{},
	}
};
