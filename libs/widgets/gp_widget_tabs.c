//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>

#include <gp_widgets.h>
#include <gp_widget_ops.h>
#include <gp_string.h>

static gp_size tab_w(gp_widget *self, const gp_widget_render_ctx *ctx,
                     unsigned int tab)
{
	const char *label = self->tabs->labels[tab];

	return gp_text_width(ctx->font_bold, label) + 2 * ctx->padd;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int i, max_min_w = 0, tabs_width = 0;

	for (i = 0; i < self->tabs->count; i++) {
		unsigned int min_w = gp_widget_min_w(self->tabs->widgets[i], ctx);
		max_min_w = GP_MAX(max_min_w, min_w);

		tabs_width += tab_w(self, ctx, i);
	}

	return GP_MAX(max_min_w + 2 * ctx->padd, tabs_width);
}

static gp_size title_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void) self;

	return gp_text_ascent(ctx->font) + 2 * ctx->padd;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int i, max_min_h = 0;

	for (i = 0; i < self->tabs->count; i++) {
		unsigned int min_h = gp_widget_min_h(self->tabs->widgets[i], ctx);

		max_min_h = GP_MAX(max_min_h, min_h);
	}

	return max_min_h + title_h(self, ctx) + 2 * ctx->padd;
}

static gp_size payload_x(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	return self->x + ctx->padd;
}

static gp_size payload_y(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	return self->y + title_h(self, ctx) + ctx->padd;
}

static gp_size payload_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	return self->w - 2 * ctx->padd;
}

static gp_size payload_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	return self->h - title_h(self, ctx) - 2 * ctx->padd;
}

static void distribute_size(gp_widget *self, const gp_widget_render_ctx *ctx,
                            int new_wh)
{
	unsigned int i;
	unsigned int w = payload_w(self, ctx);
	unsigned int h = payload_h(self, ctx);

	for (i = 0; i < self->tabs->count; i++) {
		gp_widget *widget = self->tabs->widgets[i];

		if (!widget)
			continue;

		gp_widget_ops_distribute_size(widget, ctx, w, h, new_wh);
	}
}

static int active_first(gp_widget *self)
{
	return self->tabs->active_tab == 0;
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int i;
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int tab_h = title_h(self, ctx);
	unsigned int act_x = 0, act_w = 0;

	gp_widget *widget = self->tabs->widgets[self->tabs->active_tab];

	if (self->redraw)
		gp_widget_ops_blit(ctx, x, y, self->w, self->h);

	if (!widget) {
		gp_fill_rect_xywh(ctx->buf, x, y,
				  self->w, self->h, ctx->bg_color);
	} else {
		/* Fill in area from top up to the widget inside */
		gp_fill_rect_xywh(ctx->buf, x, y,
		                  self->w, tab_h + ctx->padd + widget->y, ctx->bg_color);
	}

	unsigned int cur_x = x;

	for (i = 0; i < self->tabs->count; i++) {
		const char *label = self->tabs->labels[i];
		int is_active = self->tabs->active_tab == i;
		gp_text_style *font = is_active ? ctx->font_bold : ctx->font;

		unsigned int w = gp_text_width(ctx->font_bold, label) + 2 * ctx->padd;

		if (is_active) {
			act_x = cur_x;
			act_w = w;
		}

		if (is_active && self->tabs->title_focused) {
			gp_hline_xyw(ctx->buf,
				    cur_x + ctx->padd/2,
				    y + tab_h - ctx->padd,
				    w - ctx->padd, ctx->sel_color);
		}

		gp_text(ctx->buf, font, cur_x + w/2, y + ctx->padd,
			GP_ALIGN_CENTER|GP_VALIGN_BELOW,
			ctx->text_color, ctx->bg_color, label);

		cur_x += w;

		if (cur_x < x + self->w)
			gp_vline_xyh(ctx->buf, cur_x-1, y+1, tab_h-1, ctx->text_color);
	}

	if (!active_first(self))
		gp_hline_xxy(ctx->buf, x, act_x-1, y + tab_h, ctx->text_color);

	gp_hline_xxy(ctx->buf, act_x + act_w - 1, x + self->w-1, y + tab_h, ctx->text_color);

	gp_rrect_xywh(ctx->buf, x, y, self->w, self->h, ctx->text_color);

	if (!widget)
		return;

	int spy = y + tab_h + ctx->padd;

	gp_fill_rect_xyxy(ctx->buf, x + 1, spy + widget->y + widget->h,
	                  x + self->w - 2, y + self->h-2, ctx->bg_color);

	gp_fill_rect_xywh(ctx->buf, x + widget->x + widget->w + ctx->padd, spy + widget->y,
	                  self->w - widget->x - widget->w - ctx->padd - 1, widget->h, ctx->bg_color);

	gp_fill_rect_xywh(ctx->buf, x + 1, spy + widget->y,
	                  widget->x + ctx->padd - 1, widget->h, ctx->bg_color);

	gp_coord px = payload_x(self, ctx) + offset->x;
	gp_coord py = payload_y(self, ctx) + offset->y;

	gp_offset widget_offset = {
		.x = px,
		.y = py,
	};

	gp_widget_ops_render(widget, &widget_offset, ctx, flags);
}

static void set_tab(gp_widget *self, unsigned int tab)
{
	if (tab == self->tabs->active_tab)
		return;

	self->tabs->active_tab = tab;
	gp_widget_redraw(self);
	gp_widget_redraw_children(self);
}

static gp_widget *active_tab_widget(gp_widget *self)
{
	return self->tabs->widgets[self->tabs->active_tab];
}

static void tab_left(gp_widget *self)
{
	unsigned int tab;

	if (self->tabs->active_tab > 0)
		tab = self->tabs->active_tab - 1;
	else
		tab = self->tabs->count - 1;

	set_tab(self, tab);
}

static void tab_right(gp_widget *self)
{
	unsigned int tab;

	if (self->tabs->active_tab + 1 < self->tabs->count)
		tab = self->tabs->active_tab + 1;
	else
		tab = 0;

	set_tab(self, tab);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	if (self->tabs->widget_focused) {
		unsigned int px = payload_x(self, ctx) - self->x;
		unsigned int py = payload_y(self, ctx) - self->y;

		return gp_widget_ops_event_offset(active_tab_widget(self), ctx, ev, px, py);
	}

	if (!self->tabs->title_focused)
		return 0;

	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		switch (ev->val) {
		case GP_KEY_LEFT:
			tab_left(self);
			return 1;
		break;
		case GP_KEY_RIGHT:
			tab_right(self);
			return 1;
		break;
		}
	break;
	}

	return 0;
}

static int focus_out(gp_widget *self)
{
	if (self->tabs->widget_focused)
		return gp_widget_ops_render_focus(active_tab_widget(self), GP_FOCUS_OUT);

	if (self->tabs->title_focused) {
		self->tabs->title_focused = 0;
		gp_widget_redraw(self);
	}

	return 0;
}

static int focus_prev(gp_widget *self)
{
	gp_widget *w = active_tab_widget(self);

	if (self->tabs->title_focused)
		return 0;

	if (self->tabs->widget_focused) {
		gp_widget_ops_render_focus(w, GP_FOCUS_OUT);
		self->tabs->widget_focused = 0;
		self->tabs->title_focused = 1;
		gp_widget_redraw(self);
		return 1;
	}

	if (gp_widget_ops_render_focus(w, GP_FOCUS_IN))
		return 1;

	self->tabs->title_focused = 1;
	gp_widget_redraw(self);
	return 1;
}

static int focus_next(gp_widget *self)
{
	gp_widget *w = active_tab_widget(self);

	if (self->tabs->title_focused) {
		if (!gp_widget_ops_render_focus(w, GP_FOCUS_IN))
			return 0;
		self->tabs->title_focused = 0;
		self->tabs->widget_focused = 1;
		gp_widget_redraw(self);
		return 1;
	}

	if (self->tabs->widget_focused)
		return 0;

	self->tabs->title_focused = 1;
	gp_widget_redraw(self);
	return 1;
}

static int focus(gp_widget *self, int sel)
{
	gp_widget *w = active_tab_widget(self);

	if (self->tabs->widget_focused) {
		if (gp_widget_ops_render_focus(w, sel))
			return 1;
	}

	switch (sel) {
	case GP_FOCUS_OUT:
		return focus_out(self);
	case GP_FOCUS_LEFT:
		return 0;
	case GP_FOCUS_RIGHT:
		return 0;
	case GP_FOCUS_UP:
	case GP_FOCUS_PREV:
		return focus_prev(self);
	case GP_FOCUS_DOWN:
	case GP_FOCUS_NEXT:
		return focus_next(self);
	}

	return 0;
}

static void focus_tab(gp_widget *self, const gp_widget_render_ctx *ctx,
                       unsigned int x)
{
	unsigned int i, cx = 0;

	for (i = 0; i < self->tabs->count; i++) {
		unsigned int w = tab_w(self, ctx, i);

		if (x <= cx + w)
			break;

		cx += w;
	}

	if (i == self->tabs->count)
		return;

	set_tab(self, i);
}


static int focus_title(gp_widget *self, const gp_widget_render_ctx *ctx,
                        unsigned int x)
{
	self->tabs->title_focused = 1;

	if (self->tabs->widget_focused) {
		gp_widget_ops_render_focus(active_tab_widget(self), GP_FOCUS_OUT);
		self->tabs->widget_focused = 0;
	}

	focus_tab(self, ctx, x);

	return 1;
}

static int focus_widget(gp_widget *self, const gp_widget_render_ctx *ctx,
                         unsigned int x, unsigned int y)
{
	if (!gp_widget_ops_render_focus_xy(active_tab_widget(self), ctx, x, y))
		return 0;

	if (self->tabs->title_focused) {
		self->tabs->title_focused = 0;
		gp_widget_redraw(self);
	}

	self->tabs->widget_focused = 1;
	return 1;
}

static int focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                    unsigned int x, unsigned int y)
{
	if (y > title_h(self, ctx))
		return focus_widget(self, ctx, x - payload_x(self, ctx) + self->x, y - payload_y(self, ctx) + self->y);

	return focus_title(self, ctx, x);
}

static gp_widget *json_to_tabs(json_object *json, void **uids)
{
	json_object *widgets = NULL;
	json_object *labels = NULL;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "labels"))
			labels = val;
		else if (!strcmp(key, "widgets"))
			widgets = val;
		else
			GP_WARN("Invalid tabs key '%s'", key);
	}

	if (!labels) {
		GP_WARN("Missing tabs array!");
		return NULL;
	}

	if (!widgets) {
		GP_WARN("Missing widgets array!");
		return NULL;
	}

	if (!json_object_is_type(labels, json_type_array)) {
		GP_WARN("Tabs has to be array of strings!");
		return NULL;
	}

	if (!json_object_is_type(widgets, json_type_array)) {
		GP_WARN("Tabs has to be array of strings!");
		return NULL;
	}

	unsigned int i, tab_count = json_object_array_length(labels);
	const char *tab_labels[tab_count];

	for (i = 0; i < tab_count; i++) {
		json_object *label = json_object_array_get_idx(labels, i);
		tab_labels[i] = json_object_get_string(label);

		if (!tab_labels[i])
			GP_WARN("Tab title %i must be string!", i);
	}

	gp_widget *ret = gp_widget_tabs_new(tab_count, 0, tab_labels);

	for (i = 0; i < tab_count; i++) {
		json_object *json_widget = json_object_array_get_idx(widgets, i);

		if (!json_widget) {
			GP_WARN("Not enough widgets to fill tabs!");
			return ret;
		}

		ret->tabs->widgets[i] = gp_widget_from_json(json_widget, uids);

		gp_widget_set_parent(ret->tabs->widgets[i], ret);
	}


	return ret;
}

static void for_each_child(gp_widget *self, void (*func)(gp_widget *child))
{
	unsigned int i;

	for (i = 0; i < self->tabs->count; i++) {
		gp_widget *child = self->tabs->widgets[i];

		if (child)
			func(child);
	}
}

struct gp_widget_ops gp_widget_tabs_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.focus = focus,
	.focus_xy = focus_xy,
	.distribute_size = distribute_size,
	.for_each_child = for_each_child,
	.from_json = json_to_tabs,
	.id = "tabs",
};

gp_widget *gp_widget_tabs_new(unsigned int tabs, unsigned int active_tab,
                              const char *tab_labels[])
{
	size_t size = sizeof(struct gp_widget_tabs) + tabs * sizeof(void*);

	size += gp_string_arr_size(tab_labels, tabs);

	gp_widget *ret = gp_widget_new(GP_WIDGET_TABS, GP_WIDGET_CLASS_NONE, size);
	if (!ret)
		return NULL;

	memset(ret->tabs, 0, size);

	if (active_tab >= tabs) {
		GP_WARN("Active tab %u >= tabs %u", active_tab, tabs);
		active_tab = 0;
	}

	void *payload = ret->tabs->payload;

	ret->tabs->count = tabs;
	ret->tabs->active_tab = active_tab;
	ret->tabs->widgets = payload;
	payload += tabs * sizeof(void*);
	ret->tabs->labels = gp_string_arr_copy(tab_labels, tabs, payload);

	return ret;
}

gp_widget *gp_widget_tabs_put(gp_widget *self, unsigned int tab,
                              gp_widget *child)
{
	gp_widget *ret;

	GP_WIDGET_ASSERT(self, GP_WIDGET_TABS, NULL);

	if (tab >= self->tabs->count) {
		GP_WARN("Invalid tabs index %u", tab);
		return NULL;
	}

	ret = self->tabs->widgets[tab];
	if (ret)
		ret->parent = NULL;

	self->tabs->widgets[tab] = child;

	gp_widget_set_parent(child, self);

	gp_widget_resize(self);
	//TODO: Redraw as well?

	return ret;
}
