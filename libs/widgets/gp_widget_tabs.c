//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_vec.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_string.h>

struct gp_widget_tab {
	char *label;
	gp_widget *widget;
};

struct gp_widget_tabs {
	unsigned int active_tab;

	int title_focused:1;
	int widget_focused:1;

	struct gp_widget_tab *tabs;

	char payload[];
};

static gp_size tab_w(gp_widget *self, const gp_widget_render_ctx *ctx,
                     unsigned int tab)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);
	const char *label = tabs->tabs[tab].label;

	return gp_text_wbbox(ctx->font_bold, label) + 2 * ctx->padd;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);
	unsigned int i, max_min_w = 0, tabs_width = 0;

	for (i = 0; i < gp_vec_len(tabs->tabs); i++) {
		unsigned int min_w = gp_widget_min_w(tabs->tabs[i].widget, ctx);
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
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);
	unsigned int i, max_min_h = 0;

	for (i = 0; i < gp_vec_len(tabs->tabs); i++) {
		unsigned int min_h = gp_widget_min_h(tabs->tabs[i].widget, ctx);

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

static void distribute_w(gp_widget *self, const gp_widget_render_ctx *ctx,
                         int new_wh)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);
	unsigned int w = payload_w(self, ctx);
	unsigned int i;

	for (i = 0; i < gp_vec_len(tabs->tabs); i++) {
		gp_widget *widget = tabs->tabs[i].widget;

		if (!widget)
			continue;

		gp_widget_ops_distribute_w(widget, ctx, w, new_wh);
	}
}

static void distribute_h(gp_widget *self, const gp_widget_render_ctx *ctx,
                         int new_wh)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);
	unsigned int h = payload_h(self, ctx);
	unsigned int i;

	for (i = 0; i < gp_vec_len(tabs->tabs); i++) {
		gp_widget *widget = tabs->tabs[i].widget;

		if (!widget)
			continue;

		gp_widget_ops_distribute_h(widget, ctx, h, new_wh);
	}
}

static int active_first(gp_widget *self)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	return tabs->active_tab == 0;
}

static gp_widget *active_tab_widget(gp_widget *self)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (!gp_vec_len(tabs->tabs))
		return NULL;

	return tabs->tabs[tabs->active_tab].widget;
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int tab_h = title_h(self, ctx);
	unsigned int act_x = 0, act_w = 0;
	unsigned int i;
	gp_pixel text_color = ctx->text_color;

	if (gp_widget_is_disabled(self, flags))
		text_color = ctx->col_disabled;

	if (self->redraw)
		gp_widget_ops_blit(ctx, x, y, self->w, self->h);

	if (!gp_vec_len(tabs->tabs)) {
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

	for (i = 0; i < gp_vec_len(tabs->tabs); i++) {
		const char *label = tabs->tabs[i].label;
		int is_active = tabs->active_tab == i;
		gp_text_style *font = is_active ? ctx->font_bold : ctx->font;

		unsigned int w = gp_text_wbbox(ctx->font_bold, label) + 2 * ctx->padd;

		if (is_active) {
			act_x = cur_x;
			act_w = w;
		}

		if (is_active && tabs->title_focused) {
			gp_fill_rect_xywh(ctx->buf,
				    cur_x + ctx->padd/2,
				    y + tab_h - ctx->padd + 1,
				    w - ctx->padd, ctx->fr_thick+1,
			            ctx->sel_color);
		}

		gp_text(ctx->buf, font, cur_x + w/2, y + ctx->padd,
			GP_ALIGN_CENTER|GP_VALIGN_BELOW,
			text_color, ctx->bg_color, label);

		cur_x += w;

		if (cur_x < x + self->w)
			gp_fill_rect_xywh(ctx->buf, cur_x-(ctx->fr_thick+1)/2, y+1, ctx->fr_thick, tab_h-1, text_color);
	}

	gp_size fr_th_20 = ctx->fr_thick/2;
	gp_size fr_th_21 = (ctx->fr_thick+1)/2;

	if (!active_first(self))
		gp_fill_rect_xyxy(ctx->buf, x, y + tab_h, act_x+fr_th_20-1, y + tab_h + ctx->fr_thick-1, text_color);

	gp_fill_rect_xyxy(ctx->buf, act_x + act_w - fr_th_21, y + tab_h, x + self->w-1, y + tab_h + ctx->fr_thick-1, text_color);

	if (widget) {
		gp_coord spy = y + tab_h + ctx->padd;

		gp_fill_rect_xyxy(ctx->buf, x, spy + widget->y + widget->h,
		                  x + self->w - 1, y + self->h - 1, ctx->bg_color);

		gp_fill_rect_xywh(ctx->buf, x + widget->x + widget->w + ctx->padd, spy + widget->y,
		                  self->w - widget->x - widget->w - ctx->padd - 1, widget->h, ctx->bg_color);

		gp_fill_rect_xywh(ctx->buf, x + 1, spy + widget->y,
		                  widget->x + ctx->padd - 1, widget->h, ctx->bg_color);
	}

	gp_rrect_xywh(ctx->buf, x, y, self->w, self->h, text_color);

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
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (tab == tabs->active_tab)
		return;

	gp_widget_send_widget_event(self, GP_WIDGET_TABS_DEACTIVATED);
	tabs->active_tab = tab;
	gp_widget_send_widget_event(self, GP_WIDGET_TABS_ACTIVATED);

	gp_widget_redraw(self);
	gp_widget_redraw_children(self);
}

static void tab_left(gp_widget *self)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	unsigned int tab;

	if (tabs->active_tab > 0)
		tab = tabs->active_tab - 1;
	else
		tab = gp_vec_len(tabs->tabs) - 1;

	set_tab(self, tab);
}

static void tab_right(gp_widget *self)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	unsigned int tab;

	if (tabs->active_tab + 1 < gp_vec_len(tabs->tabs))
		tab = tabs->active_tab + 1;
	else
		tab = 0;

	set_tab(self, tab);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (tabs->widget_focused) {
		unsigned int px = payload_x(self, ctx) - self->x;
		unsigned int py = payload_y(self, ctx) - self->y;

		return gp_widget_ops_event_offset(active_tab_widget(self), ctx, ev, px, py);
	}

	if (!tabs->title_focused)
		return 0;

	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		if (gp_widget_key_mod_pressed(ev))
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
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (tabs->widget_focused)
		return gp_widget_ops_render_focus(active_tab_widget(self), GP_FOCUS_OUT);

	if (tabs->title_focused) {
		tabs->title_focused = 0;
		gp_widget_redraw(self);
	}

	return 0;
}

static int focus_prev(gp_widget *self)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	gp_widget *w = active_tab_widget(self);

	if (tabs->title_focused)
		return 0;

	if (tabs->widget_focused) {
		gp_widget_ops_render_focus(w, GP_FOCUS_OUT);
		tabs->widget_focused = 0;
		tabs->title_focused = 1;
		gp_widget_redraw(self);
		return 1;
	}

	if (gp_widget_ops_render_focus(w, GP_FOCUS_IN))
		return 1;

	tabs->title_focused = 1;
	gp_widget_redraw(self);
	return 1;
}

static int focus_next(gp_widget *self)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	gp_widget *w = active_tab_widget(self);

	if (tabs->title_focused) {
		if (!gp_widget_ops_render_focus(w, GP_FOCUS_IN))
			return 0;
		tabs->title_focused = 0;
		tabs->widget_focused = 1;
		gp_widget_redraw(self);
		return 1;
	}

	if (tabs->widget_focused)
		return 0;

	tabs->title_focused = 1;
	gp_widget_redraw(self);
	return 1;
}

static int focus(gp_widget *self, int sel)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	gp_widget *w = active_tab_widget(self);

	if (tabs->widget_focused) {
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
	case GP_FOCUS_IN:
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
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	unsigned int i, cx = 0;

	for (i = 0; i < gp_vec_len(tabs->tabs); i++) {
		unsigned int w = tab_w(self, ctx, i);

		if (x <= cx + w)
			break;

		cx += w;
	}

	if (i == gp_vec_len(tabs->tabs))
		return;

	set_tab(self, i);
}


static int focus_title(gp_widget *self, const gp_widget_render_ctx *ctx,
                        unsigned int x)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	tabs->title_focused = 1;

	if (tabs->widget_focused) {
		gp_widget_ops_render_focus(active_tab_widget(self), GP_FOCUS_OUT);
		tabs->widget_focused = 0;
	}

	focus_tab(self, ctx, x);

	return 1;
}

static void unfocus_title(gp_widget *self)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (!tabs->title_focused)
		return;

	tabs->title_focused = 0;
	gp_widget_redraw(self);
}

static int focus_widget(gp_widget *self, const gp_widget_render_ctx *ctx,
                         unsigned int x, unsigned int y)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (!gp_widget_ops_render_focus_xy(active_tab_widget(self), ctx, x, y))
		return 0;

	unfocus_title(self);
	tabs->widget_focused = 1;

	return 1;
}

static int focus_child(gp_widget *self, gp_widget *child)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	gp_widget *active = active_tab_widget(self);

	if (child != active)
		return 0;

	unfocus_title(self);
	tabs->widget_focused = 1;
	return 1;
}

static int focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                    unsigned int x, unsigned int y)
{
	if (y > title_h(self, ctx))
		return focus_widget(self, ctx, x - payload_x(self, ctx) + self->x, y - payload_y(self, ctx) + self->y);

	return focus_title(self, ctx, x);
}

static void tabs_free(struct gp_widget_tab *tabs)
{
	size_t i;

	for (i = 0; i < gp_vec_len(tabs); i++) {
		free(tabs->label);
		gp_widget_free(tabs->widget);
	}
}

static gp_widget *tabs_new(struct gp_widget_tab *tabs_arr, unsigned int active_tab)
{
	size_t size = sizeof(struct gp_widget_tabs);

	gp_widget *ret = gp_widget_new(GP_WIDGET_TABS, GP_WIDGET_CLASS_NONE, size);
	if (!ret) {
		tabs_free(tabs_arr);
		return NULL;
	}

	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(ret);

	tabs->tabs = tabs_arr;
	tabs->active_tab = active_tab;

	gp_widget_send_widget_event(ret, GP_WIDGET_TABS_ACTIVATED);

	return ret;
}

enum keys {
	ACTIVE,
	LABELS,
	WIDGETS,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("active", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("labels", GP_JSON_ARR),
	GP_JSON_OBJ_ATTR("widgets", GP_JSON_ARR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static int parse_labels(gp_json_reader *json, gp_json_val *val, struct gp_widget_tab **tabs)
{
	struct gp_widget_tab *tmp;
	size_t idx;

	GP_JSON_ARR_FOREACH(json, val) {
		switch (val->type) {
		case GP_JSON_STR:
			idx = gp_vec_len(*tabs);
			tmp = gp_vec_expand(*tabs, 1);
			if (!tmp) {
				gp_json_err(json, "Allocation failure");
				return 1;
			}
			*tabs = tmp;
			tmp[idx].label = strdup(val->val_str);
		break;
		default:
			gp_json_err(json, "Invalid label type");
			return 1;
		}
	}

	return 0;
}

static int parse_widgets(gp_json_reader *json, gp_json_val *val,
                         struct gp_widget_tab *tabs, gp_widget_json_ctx *ctx)
{
	size_t idx = 0;
	int warned = 0;

	GP_JSON_ARR_FOREACH(json, val) {
		switch (val->type) {
		case GP_JSON_OBJ:
			if (idx >= gp_vec_len(tabs)) {
				if (!warned) {
					gp_json_warn(json, "Too many widgets!");
					warned = 1;
				}

				gp_json_obj_skip(json);
			} else {
				tabs[idx++].widget = gp_widget_from_json(json, val, ctx);
			}
		break;
		default:
			gp_json_warn(json, "Invalid widget, must be object type!");
		}
	}

	if (idx < gp_vec_len(tabs))
		gp_json_warn(json, "Not enough widgets!");

	return 0;
}

static gp_widget *json_to_tabs(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	int active = 0;
	struct gp_widget_tab *tabs;
	gp_widget *ret;
	size_t i;

	tabs = gp_vec_new(0, sizeof(struct gp_widget_tab));
	if (!tabs)
		return NULL;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case LABELS:
			if (parse_labels(json, val, &tabs))
				goto free;
		break;
		case WIDGETS:
			if (!gp_vec_len(tabs)) {
				gp_json_err(json, "Label array has to precede widgets array");
				goto free;
			}

			if (parse_widgets(json, val, tabs, ctx))
				goto free;
		break;
		case ACTIVE:
			active = val->val_int;
		break;
		}
	}

	if (active < 0 || (size_t)active >= gp_vec_len(tabs)) {
		gp_json_warn(json, "Active widget must be a valid tab index");
		active = 0;
	}

	ret = tabs_new(tabs, active);
	if (!ret)
		return NULL;

	for (i = 0; i < gp_vec_len(tabs); i++)
		gp_widget_set_parent(tabs[i].widget, ret);

	return ret;
free:
	tabs_free(tabs);
	return NULL;
}

static void for_each_child(gp_widget *self, void (*func)(gp_widget *child))
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);
	size_t i;

	for (i = 0; i < gp_vec_len(tabs->tabs); i++) {
		gp_widget *child = tabs->tabs[i].widget;

		if (child)
			func(child);
	}
}

static void free_(gp_widget *self)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);
	size_t i;

	for (i = 0; i < gp_vec_len(tabs->tabs); i++)
		free(tabs->tabs[i].label);

	gp_vec_free(tabs->tabs);
}

struct gp_widget_ops gp_widget_tabs_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.focus = focus,
	.focus_xy = focus_xy,
	.focus_child = focus_child,
	.distribute_w = distribute_w,
	.distribute_h = distribute_h,
	.for_each_child = for_each_child,
	.free = free_,
	.from_json = json_to_tabs,
	.id = "tabs",
};

gp_widget *gp_widget_tabs_new(unsigned int tab_cnt, unsigned int active_tab,
                              const char *tab_labels[], int flags)
{
	struct gp_widget_tab *tabs;
	size_t i;

	if (flags) {
		GP_WARN("flags has to be 0");
		return NULL;
	}

	tabs = gp_vec_new(tab_cnt, sizeof(struct gp_widget_tab));
	if (!tabs)
		return NULL;

	for (i = 0; i < tab_cnt; i++) {
		tabs[i].label = strdup(tab_labels[i]);
		if (!tabs[i].label)
			goto err;
	}

	if (active_tab >= tab_cnt) {
		if (tab_cnt)
			GP_WARN("Active tab %u >= tabs %u", active_tab, tab_cnt);
		active_tab = 0;
	}

	return tabs_new(tabs, active_tab);
err:
	tabs_free(tabs);
	return NULL;
}

unsigned int gp_widget_tabs_cnt(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, 0);
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	return gp_vec_len(tabs->tabs);
}

gp_widget *gp_widget_tabs_put(gp_widget *self, unsigned int tab,
                              gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, NULL);
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (tab >= gp_vec_len(tabs->tabs)) {
		GP_WARN("Invalid tabs index %u", tab);
		return NULL;
	}

	gp_widget *ret = tabs->tabs[tab].widget;
	if (ret)
		ret->parent = NULL;

	tabs->tabs[tab].widget = child;

	gp_widget_set_parent(child, self);

	gp_widget_resize(self);
	//TODO: Redraw as well?

	return ret;
}

gp_widget *gp_widget_tabs_child_get(gp_widget *self, unsigned int tab)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, NULL);
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (tab >= gp_vec_len(tabs->tabs)) {
		GP_WARN("Invalid tabs index %u", tab);
		return NULL;
	}

	return tabs->tabs[tab].widget;
}

void gp_widget_tabs_tab_ins(gp_widget *self, unsigned int tab,
                            const char *label, gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, );
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	GP_DEBUG(3, "Adding tab '%s' child %p at offset %u, tabs widget %p",
	         label, child, tab, self);

	struct gp_widget_tab *tabs_arr = gp_vec_ins(tabs->tabs, tab, 1);
	if (!tabs_arr)
		return;

	tabs->tabs = tabs_arr;

	tabs->tabs[tab].label = strdup(label);
	if (!tabs->tabs[tab].label) {
		tabs->tabs = gp_vec_del(tabs->tabs, tab, 1);
		return;
	}

	tabs->tabs[tab].widget = child;

	gp_widget_set_parent(child, self);

	gp_widget_resize(self);
	gp_widget_redraw(self);

	if (tabs->active_tab >= tab &&
	    gp_vec_len(tabs->tabs) > tabs->active_tab + 1) {
		tabs->active_tab++;
	}
}

unsigned int gp_widget_tabs_tab_append(gp_widget *self,
                                       const char *label, gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, (unsigned int)-1);
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	unsigned int ret = gp_vec_len(tabs->tabs);

	gp_widget_tabs_tab_ins(self, ret, label, child);

	return ret;
}

static gp_widget *tab_rem(gp_widget *self, unsigned int tab)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	gp_widget *ret = tabs->tabs[tab].widget;

	GP_DEBUG(3, "Removing tab %u (%s) child %p, tabs widget %p",
	         tab, tabs->tabs[tab].label, ret, self);

	free(tabs->tabs[tab].label);

	tabs->tabs = gp_vec_del(tabs->tabs, tab, 1);

	int was_active = tabs->active_tab == tab;

	if (was_active)
		gp_widget_send_widget_event(self, GP_WIDGET_TABS_DEACTIVATED);

	if (tabs->active_tab &&
	    tabs->active_tab >= tab) {
		tabs->active_tab--;
	}

	if (was_active)
		gp_widget_send_widget_event(self, GP_WIDGET_TABS_ACTIVATED);

	gp_widget_redraw(self);

	return ret;
}

gp_widget *gp_widget_tabs_tab_rem(gp_widget *self, unsigned int tab)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, NULL);
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (tab >= gp_vec_len(tabs->tabs)) {
		GP_BUG("Invalid tab index %u tabs (%p) count %zu",
		       tab, self, gp_vec_len(tabs->tabs));
		return NULL;
	}

	return tab_rem(self, tab);
}

unsigned int gp_widget_tabs_active_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, 0);
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	return tabs->active_tab;
}

gp_widget *gp_widget_tabs_active_child_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, NULL);
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (!gp_vec_len(tabs->tabs))
		return NULL;

	return active_tab_widget(self);
}

void gp_widget_tabs_active_set(gp_widget *self, unsigned int tab)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, );
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (tab == tabs->active_tab)
		return;

	if (tab >= gp_vec_len(tabs->tabs)) {
		GP_BUG("Invalid tab index %u tabs (%p) count %zu",
		       tab, self, gp_vec_len(tabs->tabs));
	}

	set_tab(self, tab);
}

void gp_widget_tabs_active_set_rel(gp_widget *self, int dir, int wrap_around)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, );
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (wrap_around > 1) {
		GP_BUG("Invalid wrap_around value!");
		return;
	}

	int64_t tab = (int64_t)tabs->active_tab + dir;
	int64_t tabs_cnt = gp_vec_len(tabs->tabs);

	if (tab < 0) {
		if (wrap_around) {
			tab %= tabs_cnt;
			tab += tabs_cnt;
		} else {
			tab = 0;
		}
	}

	if (tab >= tabs_cnt) {
		if (wrap_around)
			tab %= tabs_cnt;
		else
			tab = tabs_cnt - 1;
	}

	set_tab(self, tab);
}

static int child_to_tab(gp_widget *self, gp_widget *child)
{
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);
	unsigned int i;

	if (!child) {
		GP_WARN("Lookup for NULL child");
		return -1;
	}

	if (child->parent != self) {
		GP_WARN("Child (%p) parent %p does not match self (%p)",
			child, child->parent, self);
	}

	for (i = 0; i < gp_vec_len(tabs->tabs); i++) {
		if (tabs->tabs[i].widget == child)
			return i;
	}

	return -1;
}

int gp_widget_tabs_tab_by_child(gp_widget *self, gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, -1);

	return child_to_tab(self, child);
}

int gp_widget_tabs_tab_rem_by_child(gp_widget *self, gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, -1);

	int tab = child_to_tab(self, child);

	if (tab < 0)
		return 1;

	tab_rem(self, tab);

	return 0;
}

const char *gp_widget_tabs_label_get(gp_widget *self, unsigned int tab)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, NULL);
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (tab >= gp_vec_len(tabs->tabs)) {
		GP_WARN("Invalid tab index %u tabs (%p) count %zu",
			tab, self, gp_vec_len(tabs->tabs));
		return NULL;
	}

	return tabs->tabs[tab].label;
}

const char *gp_widget_tabs_active_label_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABS, NULL);
	struct gp_widget_tabs *tabs = GP_WIDGET_PAYLOAD(self);

	if (!gp_vec_len(tabs->tabs))
		return NULL;

	return tabs->tabs[tabs->active_tab].label;
}
