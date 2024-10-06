//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <utils/gp_vec.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

struct layout_switch_payload {
	unsigned int active_layout;
	gp_widget **layouts;
};

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int i, max_w = 0;
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);

	for (i = 0; i < gp_widget_layout_switch_layouts(self); i++)
		max_w = GP_MAX(max_w, gp_widget_min_w(s->layouts[i], ctx));

	return max_w;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int i, max_h = 0;
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);

	for (i = 0; i < gp_widget_layout_switch_layouts(self); i++)
		max_h = GP_MAX(max_h, gp_widget_min_h(s->layouts[i], ctx));

	return max_h;
}

static void distribute_w(gp_widget *self, const gp_widget_render_ctx *ctx,
                         int new_wh)
{
	unsigned int i;
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);

	for (i = 0; i < gp_widget_layout_switch_layouts(self); i++) {
		gp_widget *widget = s->layouts[i];

		if (!widget)
			continue;

		gp_widget_ops_distribute_w(widget, ctx, self->w, new_wh);
	}
}

static void distribute_h(gp_widget *self, const gp_widget_render_ctx *ctx,
                         int new_wh)
{
	unsigned int i;
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);

	for (i = 0; i < gp_widget_layout_switch_layouts(self); i++) {
		gp_widget *widget = s->layouts[i];

		if (!widget)
			continue;

		gp_widget_ops_distribute_h(widget, ctx, self->h, new_wh);
	}
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);
	gp_widget *widget = s->layouts[s->active_layout];

	return gp_widget_ops_event_offset(widget, ctx, ev, 0, 0);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	gp_widget *layout = gp_widget_layout_switch_active(self);

	gp_widget_ops_blit(ctx, x, y, w, h);

	if (!layout) {
		gp_fill_rect_xywh(ctx->buf, self->x, self->y, self->w, self->h, ctx->bg_color);
		return;
	}

	gp_offset child_offset = {
		.x = x,
		.y = y,
	};

	gp_fill_rect_xywh(ctx->buf, x, y, layout->x, self->h, ctx->bg_color);
	gp_fill_rect_xywh(ctx->buf, x + layout->x + layout->w, y,
	                  self->w - layout->x - layout->w, self->h, ctx->bg_color);
	gp_fill_rect_xywh(ctx->buf, x + layout->x, y, layout->w, layout->y, ctx->bg_color);
	gp_fill_rect_xywh(ctx->buf, x + layout->x, y + layout->y + layout->h,
	                  layout->w, self->h - layout->y - layout->h, ctx->bg_color);

	gp_widget_ops_render(layout, &child_offset, ctx, flags);
}

enum keys {
	WIDGETS,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(WIDGETS, "widgets", GP_JSON_ARR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_switch(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	gp_widget *ret, *child;
	unsigned int cnt = 0;
	void *tmp;

	ret = gp_widget_layout_switch_new(0);
	if (!ret)
		return NULL;

	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(ret);

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case WIDGETS:
			GP_JSON_ARR_FOREACH(json, val) {
				child = gp_widget_from_json(json, val, ctx);
				if (!child)
					continue;

				tmp = gp_vec_expand(s->layouts, 1);
				if (!tmp) {
					gp_widget_free(child);
					continue;
				}

				s->layouts = tmp;
				s->layouts[cnt++] = child;
				gp_widget_set_parent(child, ret);
			}
		break;
		}
	}

	return ret;
}

static void free_(gp_widget *self)
{
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);

	gp_vec_free(s->layouts);
}

static void for_each_child(gp_widget *self, void (*func)(gp_widget *child))
{
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);
	unsigned int i;

	for (i = 0; i < gp_widget_layout_switch_layouts(self); i++) {
		gp_widget *child = s->layouts[i];

		if (child)
			func(child);
	}
}

static int focus(gp_widget *self, int sel)
{
	return gp_widget_ops_render_focus(gp_widget_layout_switch_active(self), sel);
}

static int focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                     unsigned int x, unsigned int y)
{
	return gp_widget_ops_render_focus_xy(gp_widget_layout_switch_active(self), ctx, x, y);
}

static int focus_child(gp_widget *self, gp_widget *child)
{
	return child == gp_widget_layout_switch_active(self);
}

struct gp_widget_ops gp_widget_layout_switch_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.distribute_w = distribute_w,
	.distribute_h = distribute_h,
	.for_each_child = for_each_child,
	.event = event,
	.focus = focus,
	.focus_xy = focus_xy,
	.focus_child = focus_child,
	.free = free_,
	.render = render,
	.from_json = json_to_switch,
	.id = "layout_switch",
};

gp_widget *gp_widget_layout_switch_new(unsigned int layouts)
{
	gp_widget *ret;

	ret = gp_widget_new(GP_WIDGET_LAYOUT_SWITCH, GP_WIDGET_CLASS_NONE, sizeof(struct layout_switch_payload));
	if (!ret)
		return NULL;

	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(ret);

	s->active_layout = 0;
	s->layouts = gp_vec_new(layouts, sizeof(gp_widget*));

	if (!s->layouts) {
		free(ret);
		return NULL;
	}

	return ret;
}

gp_widget *gp_widget_layout_switch_active(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LAYOUT_SWITCH, NULL);
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);

	return s->layouts[s->active_layout];
}

unsigned int gp_widget_layout_switch_layouts(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LAYOUT_SWITCH, 0);
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);

	return gp_vec_len(s->layouts);
}

void gp_widget_layout_switch_move(gp_widget *self, int where)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LAYOUT_SWITCH, );
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);
	int layouts = gp_vec_len(s->layouts);

	int switch_to = ((int)s->active_layout + where) % layouts;

	if (switch_to < 0)
		switch_to += layouts;

	gp_widget_layout_switch_layout(self, switch_to);
}

gp_widget *gp_widget_layout_switch_put(gp_widget *self, unsigned int layout_nr,
                                gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LAYOUT_SWITCH, NULL);
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);
	gp_widget *ret;

	if (layout_nr >= gp_widget_layout_switch_layouts(self))
		return NULL;

	ret = s->layouts[layout_nr];
	s->layouts[layout_nr] = child;

	gp_widget_set_parent(child, self);

	gp_widget_resize(self);

	return ret;
}

void gp_widget_layout_switch_layout(gp_widget *self, unsigned int layout_nr)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LAYOUT_SWITCH, );
	struct layout_switch_payload *s = GP_WIDGET_PAYLOAD(self);

	if (layout_nr >= gp_widget_layout_switch_layouts(self)) {
		GP_WARN("Invalid layout nr %i", layout_nr);
		return;
	}

	s->active_layout = layout_nr;

	gp_widget_redraw_children(self);
}
