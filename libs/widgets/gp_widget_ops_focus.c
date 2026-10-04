//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2026 Cyril Hrubis <metan@ucw.cz>

 */

#include <core/gp_debug.h>
#include <widgets/gp_widget.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_ops_focus.h>

static gp_widget *focus_child_get(gp_widget *self)
{
	const struct gp_widget_ops *ops = gp_widget_ops(self);

	if (!ops->focus_child_get)
		return NULL;

	return ops->focus_child_get(self);
}

static const char *focus_to_str(int flag)
{
	switch (flag) {
	case GP_FOCUS_NEXT:
		return "focus_next";
	case GP_FOCUS_PREV:
		return "focus_prev";
	case GP_FOCUS_LEFT:
		return "focus_left";
	case GP_FOCUS_RIGHT:
		return "focus_right";
	case GP_FOCUS_UP:
		return "focus_up";
	case GP_FOCUS_DOWN:
		return "focus_down";
	}

	return "???";
}

bool gp_widget_focusable(gp_widget *self)
{
	if (!self)
		return false;

	if (!gp_widget_accepts_input_events(self))
		return false;

	return true;
}

bool gp_widget_focused(gp_widget *self)
{
	const struct gp_widget_ops *ops = gp_widget_ops(self);

	if (!self->focused)
		return false;

	if (!ops->focus_child_get)
		return true;

	return ops->focus_child_get(self) == self;
}

static bool redraw_focus(gp_widget *self)
{
	return !gp_widget_ops(self)->focus_child_get || focus_child_get(self) == self;
}

static void unfocus_path(gp_widget *self)
{
	gp_widget *child;

	if (!self || !self->focused)
		return;

	child = focus_child_get(self);
	if (child && child != self)
		unfocus_path(child);

	self->focused = 0;

	gp_widget_send_event(self, GP_WIDGET_EVENT_FOCUS,
	                           GP_WIDGET_EVENT_FOCUS_OUT);

	if (redraw_focus(self))
		gp_widget_redraw(self);
}

static void focus_path(gp_widget *self, gp_widget *top)
{
	if (self == top)
		return;

	focus_path(self->parent, top);

	self->focused = 1;

	gp_widget_send_event(self, GP_WIDGET_EVENT_FOCUS,
	                           GP_WIDGET_EVENT_FOCUS_IN);

	if (redraw_focus(self))
		gp_widget_redraw(self);
}

static bool focus_child_set(gp_widget *self, gp_widget *child)
{
	const struct gp_widget_ops *ops = gp_widget_ops(self);

	GP_DEBUG(4, "Focusing child %p (%s) in parent %p (%s)",
	         child, gp_widget_type_id(child),
	         self, gp_widget_type_id(self));

	if (!ops->focus_child_set) {
		GP_WARN("Unimplemented focus_child_set() in %s!", ops->id);
		return false;
	}

	return ops->focus_child_set(self, child);
}

static gp_widget *first_focused_ancestor(gp_widget *self)
{
	while (self && !self->focused)
		self = self->parent;

	return self;
}

static bool focus_meet(gp_widget *top, gp_widget *child)
{
	gp_widget *old = focus_child_get(top);

	if (!focus_child_set(top, child))
		return false;

	if (old != top)
		unfocus_path(old);

	//TODO: Move this to the container widget?
	if (old == top || child == top)
		gp_widget_redraw(top);

	return true;
}

static gp_widget *container_or_parent(gp_widget *self)
{
	const struct gp_widget_ops *ops = gp_widget_ops(self);

	if (ops->focus_child_set)
		return self;

	return self->parent;
}

static bool focus_switch(gp_widget *self)
{
	gp_widget *top, *w, *child = self;

	if (gp_widget_focused(self))
		return true;

	top = first_focused_ancestor(self);

	for (w = container_or_parent(self); w != top; child = w, w = w->parent) {
		if (!focus_child_set(w, child))
			return false;
	}

	if (top && !focus_meet(top, child))
		return false;

	focus_path(self, top);

	return true;
}

/*
 * Returns the first widget in focus_dir inside of self that can be focused or
 * NULL. This is used when focus moves into a sibling and ignores the focused
 * children the containers remember, i.e. moving focus forward enters a
 * container at its first widget and moving backward at its last widget.
 */
static gp_widget *focus_enter(gp_widget *self, int focus_dir)
{
	const struct gp_widget_ops *ops;
	gp_widget *next, *ret;

	if (!gp_widget_focusable(self))
		return NULL;

	ops = gp_widget_ops(self);

	if (!ops->focus_child_get)
		return self->focused ? NULL : self;

	/* A container with a single child */
	if (!ops->focus_child_next) {
		next = ops->focus_child_get(self);

		if (!next || next == self)
			return next;

		return focus_enter(next, focus_dir);
	}

	for (next = ops->focus_child_next(self, NULL, focus_dir); next;
	     next = ops->focus_child_next(self, next, focus_dir)) {
		if (next == self)
			return self;

		ret = focus_enter(next, focus_dir);
		if (ret)
			return ret;
	}

	return NULL;
}

/*
 * Returns a widget that should be focused when focus is moved in focus_dir
 * inside of self or NULL if focus cannot be moved.
 *
 * The focus path is followed through the focused children down to the
 * focused widget and then moves to a sibling, as far up as needed. When self
 * isn't focused, the focus moves back into it and the remembered focused
 * children are followed, i.e. the focus returns where it was, or if that is
 * not possible, e.g. the widget got disabled, enters it at the first widget.
 */
static gp_widget *focus_find(gp_widget *self, int focus_dir)
{
	const struct gp_widget_ops *ops;
	gp_widget *child, *next, *ret;

	if (!gp_widget_focusable(self))
		return NULL;

	ops = gp_widget_ops(self);

	if (!ops->focus_child_get)
		return self->focused ? NULL : self;

	ops = gp_widget_ops(self);
	child = ops->focus_child_get(self);

	if (child == self) {
		if (!self->focused)
			return self;
	} else if (child) {
		ret = focus_find(child, focus_dir);
		if (ret)
			return ret;
	}

	if (!self->focused)
		return focus_enter(self, focus_dir);

	if (!ops->focus_child_next)
		return NULL;

	for (next = ops->focus_child_next(self, child, focus_dir); next;
	     next = ops->focus_child_next(self, next, focus_dir)) {
		if (next == self)
			return self;

		ret = focus_enter(next, focus_dir);
		if (ret)
			return ret;
	}

	return NULL;
}

/*
 * Returns a widget that should be focused on x, y or NULL.
 */
static gp_widget *focus_find_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                                unsigned int x, unsigned int y)
{
	const struct gp_widget_ops *ops;
	gp_widget *child;

	for (;;) {
		if (!gp_widget_focusable(self))
			return NULL;

		ops = gp_widget_ops(self);
		if (!ops->focus_child_xy)
			return self;

		x -= self->x;
		y -= self->y;

		child = ops->focus_child_xy(self, ctx, &x, &y);
		if (!child || child == self)
			return child;

		self = child;
	}
}

int gp_widget_ops_render_focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                                  unsigned int x, unsigned int y)
{
	gp_widget *target;

	GP_DEBUG(3, "focus event %p (%s) %ux%u",
		 self, gp_widget_type_id(self), x, y);

	if (!self)
		return 0;

	target = focus_find_xy(self, ctx, x, y);
	if (!target || gp_widget_focused(target))
		return 0;

	return focus_switch(target);
}

void gp_widget_ops_focus_child_changed(gp_widget *self, gp_widget *old_child)
{
	gp_widget *target;

	if (!self || !self->focused)
		return;

	if (!old_child || !old_child->focused)
		return;

	GP_DEBUG(3, "Focused child %p (%s) of %p (%s) changed",
	         old_child, gp_widget_type_id(old_child),
	         self, gp_widget_type_id(self));

	unfocus_path(old_child);

	/*
	 * Moves into the new child, possibly to a remembered position, or to
	 * the next widget, or to the previous one when there is nothing after.
	 */
	target = focus_find(self, GP_FOCUS_NEXT);
	if (!target)
		target = focus_find(self, GP_FOCUS_PREV);

	if (target && focus_switch(target))
		return;

	/* Container can be focused itself, e.g. tabs title */
	if (focus_switch(self))
		return;

	/* Nothing to focus in self, move the focus elsewhere in parent */
	if (self->parent)
		gp_widget_ops_focus_child_changed(self->parent, self);
	else
		unfocus_path(self);
}

int gp_widget_ops_focus_widget(gp_widget *self)
{
	if (!self)
		return 1;

	if (!gp_widget_focusable(self)) {
		GP_WARN("Widget %p (%s) not fucusable",
			self, gp_widget_type_id(self));
		return 0;
	}

	return focus_switch(self);
}

int gp_widget_focus_set(gp_widget *self)
{
	GP_DEBUG(3, "Focusing widget %p (%s)", self, gp_widget_type_id(self));

	if (!self) {
		GP_WARN("Attempting to focus NULL widget!");
		return 0;
	}

	return gp_widget_ops_focus_widget(self);
}

int gp_widget_ops_render_focus(gp_widget *self, int focus_dir)
{
	gp_widget *target;

	GP_DEBUG(3, "Focus event %p (%s) %s",
		 self, gp_widget_type_id(self), focus_to_str(focus_dir));

	if (!self)
		return 0;

	target = focus_find(self, focus_dir);
	if (!target)
		return 0;

	return focus_switch(target);
}

int gp_widget_ops_focus_out(gp_widget *self)
{
	GP_DEBUG(3, "Focus out %p (%s)", self, gp_widget_type_id(self));

	if (!self || !self->focused)
		return 0;

	unfocus_path(self);
	return 1;
}
