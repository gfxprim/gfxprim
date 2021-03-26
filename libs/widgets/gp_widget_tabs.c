//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_vec.h>

#include <gp_widgets.h>
#include <gp_widget_ops.h>
#include <gp_string.h>

static gp_size tab_w(gp_widget *self, const gp_widget_render_ctx *ctx,
                     unsigned int tab)
{
	const char *label = self->tabs->tabs[tab].label;

	return gp_text_width(ctx->font_bold, label) + 2 * ctx->padd;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int i, max_min_w = 0, tabs_width = 0;

	for (i = 0; i < gp_vec_len(self->tabs->tabs); i++) {
		unsigned int min_w = gp_widget_min_w(self->tabs->tabs[i].widget, ctx);
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

	for (i = 0; i < gp_vec_len(self->tabs->tabs); i++) {
		unsigned int min_h = gp_widget_min_h(self->tabs->tabs[i].widget, ctx);

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

	for (i = 0; i < gp_vec_len(self->tabs->tabs); i++) {
		gp_widget *widget = self->tabs->tabs[i].widget;

		if (!widget)
			continue;

		gp_widget_ops_distribute_size(widget, ctx, w, h, new_wh);
	}
}

static int active_first(gp_widget *self)
{
	return self->tabs->active_tab == 0;
}

static gp_widget *active_tab_widget(gp_widget *self)
{
	return self->tabs->tabs[self->tabs->active_tab].widget;
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int i;
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int tab_h = title_h(self, ctx);
	unsigned int act_x = 0, act_w = 0;

	if (self->redraw)
		gp_widget_ops_blit(ctx, x, y, self->w, self->h);

	if (!gp_vec_len(self->tabs->tabs)) {
		gp_fill_rect_xywh(ctx->buf, x, y,
				  self->w, self->h, ctx->bg_color);
		return;
	}

	gp_widget *widget = active_tab_widget(self);
	if (!widget) {
		gp_fill_rect_xywh(ctx->buf, x, y,
				  self->w, self->h, ctx->bg_color);
	} else {
		/* Fill in area from top up to the widget inside */
		gp_fill_rect_xywh(ctx->buf, x, y,
		                  self->w, tab_h + ctx->padd + widget->y, ctx->bg_color);
	}

	unsigned int cur_x = x;

	for (i = 0; i < gp_vec_len(self->tabs->tabs); i++) {
		const char *label = self->tabs->tabs[i].label;
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

static void tab_left(gp_widget *self)
{
	unsigned int tab;

	if (self->tabs->active_tab > 0)
		tab = self->tabs->active_tab - 1;
	else
		tab = gp_vec_len(self->tabs->tabs) - 1;

	set_tab(self, tab);
}

static void tab_right(gp_widget *self)
{
	unsigned int tab;

	if (self->tabs->active_tab + 1 < gp_vec_len(self->tabs->tabs))
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

	for (i = 0; i < gp_vec_len(self->tabs->tabs); i++) {
		unsigned int w = tab_w(self, ctx, i);

		if (x <= cx + w)
			break;

		cx += w;
	}

	if (i == gp_vec_len(self->tabs->tabs))
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
	int active = 0;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "labels"))
			labels = val;
		else if (!strcmp(key, "widgets"))
			widgets = val;
		else if (!strcmp(key, "active"))
			active = json_object_get_int(val);
		else
			GP_WARN("Invalid tabs key '%s'", key);
	}

	if (active < 0) {
		GP_WARN("Active widget must be >= 0");
		active = 0;
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

	gp_widget *ret = gp_widget_tabs_new(tab_count, active, tab_labels, 0);

	for (i = 0; i < tab_count; i++) {
		json_object *json_widget = json_object_array_get_idx(widgets, i);

		if (!json_widget) {
			GP_WARN("Not enough widgets to fill tabs!");
			return ret;
		}

		ret->tabs->tabs[i].widget = gp_widget_from_json(json_widget, uids);

		gp_widget_set_parent(ret->tabs->tabs[i].widget, ret);
	}


	return ret;
}

static void for_each_child(gp_widget *self, void (*func)(gp_widget *child))
{
	size_t i;

	for (i = 0; i < gp_vec_len(self->tabs->tabs); i++) {
		gp_widget *child = self->tabs->tabs[i].widget;

		if (child)
			func(child);
	}
}

static void free_(gp_widget *self)
{
	size_t i;

	for (i = 0; i < gp_vec_len(self->tabs->tabs); i++)
		free(self->tabs->tabs[i].label);

	gp_vec_free(self->tabs->tabs);
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
	.free = free_,
	.from_json = json_to_tabs,
	.id = "tabs",
};

gp_widget *gp_widget_tabs_new(unsigned int tabs, unsigned int active_tab,
                              const char *tab_labels[], int flags)
{
	size_t i, size = sizeof(struct gp_widget_tabs);

	if (flags) {
		GP_WARN("flags has to be 0");
		return NULL;
	}

	gp_widget *ret = gp_widget_new(GP_WIDGET_TABS, GP_WIDGET_CLASS_NONE, size);
	if (!ret)
		return NULL;

	ret->tabs->tabs = gp_vec_new(tabs, sizeof(struct gp_widget_tab));

	if (!ret->tabs->tabs) {
		free(ret);
		return NULL;
	}

	for (i = 0; i < tabs; i++) {
		ret->tabs->tabs[i].label = strdup(tab_labels[i]);
		if (!ret->tabs->tabs[i].label)
			goto err;
	}

	if (active_tab >= tabs) {
		if (tabs)
			GP_WARN("Active tab %u >= tabs %u", active_tab, tabs);
		active_tab = 0;
	}

	ret->tabs->active_tab = active_tab;

	return ret;
err:
	for (i = 0; i < tabs; i++)
		free(ret->tabs->tabs[i].label);

	gp_vec_free(ret->tabs->tabs);

	free(ret);

	return NULL;
}

unsigned int gp_widget_tabs_cnt(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TABS, 0);

	return gp_vec_len(self->tabs->tabs);
}

gp_widget *gp_widget_tabs_put(gp_widget *self, unsigned int tab,
                              gp_widget *child)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TABS, NULL);

	if (tab >= gp_vec_len(self->tabs->tabs)) {
		GP_WARN("Invalid tabs index %u", tab);
		return NULL;
	}

	gp_widget *ret = self->tabs->tabs[tab].widget;
	if (ret)
		ret->parent = NULL;

	self->tabs->tabs[tab].widget = child;

	gp_widget_set_parent(child, self);

	gp_widget_resize(self);
	//TODO: Redraw as well?

	return ret;
}

gp_widget *gp_widget_tabs_get(gp_widget *self, unsigned int tab)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TABS, NULL);

	if (tab >= gp_vec_len(self->tabs->tabs)) {
		GP_WARN("Invalid tabs index %u", tab);
		return NULL;
	}

	return self->tabs->tabs[tab].widget;
}

void gp_widget_tabs_tab_ins(gp_widget *self, unsigned int tab,
                            const char *label, gp_widget *child)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TABS, );

	GP_DEBUG(3, "Adding tab '%s' child %p at offset %u, tabs widget %p",
	         label, child, tab, self);

	struct gp_widget_tab *tabs = gp_vec_insert(self->tabs->tabs, tab, 1);
	if (!tabs)
		return;

	self->tabs->tabs = tabs;

	self->tabs->tabs[tab].label = strdup(label);
	if (!self->tabs->tabs[tab].label) {
		self->tabs->tabs = gp_vec_delete(self->tabs->tabs, tab, 1);
		return;
	}

	self->tabs->tabs[tab].widget = child;

	gp_widget_set_parent(child, self);

	gp_widget_resize(self);
	gp_widget_redraw(self);

	if (self->tabs->active_tab >= tab &&
	    gp_vec_len(self->tabs->tabs) > self->tabs->active_tab + 1) {
		self->tabs->active_tab++;
	}
}

unsigned int gp_widget_tabs_tab_append(gp_widget *self,
                                       const char *label, gp_widget *child)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TABS, (unsigned int)-1);

	unsigned int ret = gp_vec_len(self->tabs->tabs);

	gp_widget_tabs_tab_ins(self, ret, label, child);

	return ret;
}

gp_widget *gp_widget_tabs_tab_rem(gp_widget *self, unsigned int tab)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TABS, NULL);

	if (tab >= gp_vec_len(self->tabs->tabs)) {
		GP_BUG("Invalid tab index %u tabs (%p) count %zu",
		       tab, self, gp_vec_len(self->tabs->tabs));
		return NULL;
	}

	gp_widget *ret = self->tabs->tabs[tab].widget;

	GP_DEBUG(3, "Removing tab %u (%s) child %p, tabs widget %p",
	         tab, self->tabs->tabs[tab].label, ret, self);

	free(self->tabs->tabs[tab].label);

	self->tabs->tabs = gp_vec_delete(self->tabs->tabs, tab, 1);

	if (self->tabs->active_tab &&
	    self->tabs->active_tab >= tab) {
		self->tabs->active_tab--;
	}

	gp_widget_redraw(self);

	return ret;
}

unsigned int gp_widget_tabs_active_get(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TABS, 0);

	return self->tabs->active_tab;
}

void gp_widget_tabs_active_set(gp_widget *self, unsigned int tab)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TABS, );

	if (tab == self->tabs->active_tab)
		return;

	if (tab >= gp_vec_len(self->tabs->tabs)) {
		GP_BUG("Invalid tab index %u tabs (%p) count %zu",
		       tab, self, gp_vec_len(self->tabs->tabs));
	}

	self->tabs->active_tab = tab;
	gp_widget_redraw(self);
}

int gp_widget_tabs_tab_by_child(gp_widget *self, gp_widget *child)
{
	unsigned int i;

	GP_WIDGET_ASSERT(self, GP_WIDGET_TABS, -1);

	for (i = 0; i < gp_vec_len(self->tabs->tabs); i++) {
		if (self->tabs->tabs[i].widget == child)
			return i;
	}

	return -1;
}
