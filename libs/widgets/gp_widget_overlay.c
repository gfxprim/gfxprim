//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <utils/gp_vec.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

struct overlay_payload_elem {
	int hidden:1;
	gp_widget *widget;
};

struct overlay_payload {
	int focused;
	struct overlay_payload_elem *stack;
};


static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int i, max_w = 0;
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	for (i = 0; i < gp_widget_overlay_stack_size(self); i++)
		max_w = GP_MAX(max_w, gp_widget_min_w(overlay->stack[i].widget, ctx));

	return max_w;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int i, max_h = 0;
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	for (i = 0; i < gp_widget_overlay_stack_size(self); i++)
		max_h = GP_MAX(max_h, gp_widget_min_h(overlay->stack[i].widget, ctx));

	return max_h;
}

static void distribute_w(gp_widget *self, const gp_widget_render_ctx *ctx,
                         int new_wh)
{
	unsigned int i;
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	for (i = 0; i < gp_widget_overlay_stack_size(self); i++) {
		gp_widget *widget = overlay->stack[i].widget;

		if (!widget)
			continue;

		gp_widget_ops_distribute_w(widget, ctx, self->w, new_wh);
	}
}

static void distribute_h(gp_widget *self, const gp_widget_render_ctx *ctx,
                         int new_wh)
{
	unsigned int i;
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	for (i = 0; i < gp_widget_overlay_stack_size(self); i++) {
		gp_widget *widget = overlay->stack[i].widget;

		if (!widget)
			continue;

		gp_widget_ops_distribute_h(widget, ctx, self->h, new_wh);
	}
}

static gp_widget *get_focused_widget(gp_widget *self)
{
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	if (overlay->focused < 0)
		return NULL;

	return overlay->stack[overlay->focused].widget;
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	gp_widget *widget = get_focused_widget(self);

	return gp_widget_ops_event_offset(widget, ctx, ev, 0, 0);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	unsigned int i;

	gp_widget_ops_blit(ctx, x, y, w, h);

	//TODO!
	//gp_fill_rect_xywh(ctx->buf, self->x, self->y, self->w, self->h, ctx->bg_color);

	gp_offset child_offset = {
		.x = x,
		.y = y,
	};

	for (i = 0; i < gp_widget_overlay_stack_size(self); i++) {
		gp_widget *widget = overlay->stack[i].widget;

		if (overlay->stack[i].hidden)
			continue;

		gp_widget_ops_render(widget, &child_offset, ctx, flags);
	}
}

enum keys {
	HIDDEN,
	WIDGETS,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(HIDDEN, "hidden", GP_JSON_ARR),
	GP_JSON_OBJ_ATTR_IDX(WIDGETS, "widgets", GP_JSON_ARR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_overlay(gp_json_reader *json, gp_json_val *val,
                                  gp_widget_json_ctx *ctx)
{
	gp_widget *ret, *child;
	unsigned int cnt = 0;
	void *tmp;

	ret = gp_widget_overlay_new(0);
	if (!ret)
		return NULL;

	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(ret);

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case HIDDEN:
			GP_JSON_ARR_FOREACH(json, val) {
				if (val->type != GP_JSON_INT) {
					gp_json_warn(json, "Wrong type, expected integer");
					continue;
				}

				if (val->val_int < 0) {
					gp_json_warn(json, "Expected positive integer");
					continue;
				}

				if ((size_t)val->val_int >= gp_vec_len(overlay->stack)) {
					gp_json_warn(json, "Position out of stack, have you defined widgets first?");
					continue;
				}

				overlay->stack[val->val_int].hidden = 1;
			}
		break;
		case WIDGETS:
			GP_JSON_ARR_FOREACH(json, val) {
				child = gp_widget_from_json(json, val, ctx);
				if (!child)
					continue;

				tmp = gp_vec_expand(overlay->stack, 1);
				if (!tmp) {
					gp_widget_free(child);
					continue;
				}

				overlay->stack = tmp;
				overlay->stack[cnt].widget = child;
				overlay->stack[cnt].hidden = 0;
				gp_widget_set_parent(child, ret);
				cnt++;
			}
		break;
		}
	}

	return ret;
}

static gp_widget *focus_widget_by_xy(gp_widget *self, unsigned int x, unsigned int y)
{
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);
	int i;

	for (i = gp_widget_overlay_stack_size(self) - 1; i > 0; i--) {
		gp_widget *widget = overlay->stack[i].widget;

		if (!widget)
			continue;

		if (x >= widget->x && y >= widget->y &&
		    x < widget->x + widget->w &&
		    y < widget->y + widget->w) {
			overlay->focused = i;
			return widget;
		}
	}

	return NULL;
}

static void free_(gp_widget *self)
{
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	gp_vec_free(overlay->stack);
}

static void for_each_child(gp_widget *self, void (*func)(gp_widget *child))
{
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);
	unsigned int i;

	for (i = 0; i < gp_widget_overlay_stack_size(self); i++) {
		gp_widget *child = overlay->stack[i].widget;

		if (child)
			func(child);
	}
}

static int focus(gp_widget *self, int sel)
{
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);
	int i;

	if (overlay->focused < 0) {
		for (i = gp_widget_overlay_stack_size(self) - 1; i > 0; i--) {
			gp_widget *widget = overlay->stack[i].widget;

			if (overlay->stack[i].hidden)
				continue;

			if (gp_widget_ops_render_focus(widget, sel)) {
				overlay->focused = i;
				return 1;
			}

		}

		return 0;
	}

	return gp_widget_ops_render_focus(get_focused_widget(self), sel);
}

static int focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                    unsigned int x, unsigned int y)
{
	gp_widget *widget = focus_widget_by_xy(self, x, y);

	if (!widget)
		return 0;

	return gp_widget_ops_render_focus_xy(widget, ctx, x, y);
}

static int focus_child(gp_widget *self, gp_widget *child)
{
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);
	int i;

	for (i = gp_widget_overlay_stack_size(self) - 1; i > 0; i--) {
		gp_widget *focused, *widget = overlay->stack[i].widget;

		if (widget != child)
			continue;

		if (overlay->stack[i].hidden) {
			GP_WARN("Attempt to focus hidden widget?!");
			return 0;
		}

		focused = get_focused_widget(self);
		if (focused)
			gp_widget_ops_render_focus(focused, GP_FOCUS_OUT);

		overlay->focused = i;
		return 1;
	}

	return 0;
}

struct gp_widget_ops gp_widget_overlay_ops = {
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
	.from_json = json_to_overlay,
	.id = "overlay",
};

gp_widget *gp_widget_overlay_new(unsigned int stack_size)
{
	gp_widget *ret;

	ret = gp_widget_new(GP_WIDGET_OVERLAY, GP_WIDGET_CLASS_NONE, sizeof(struct overlay_payload));
	if (!ret)
		return NULL;

	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(ret);

	overlay->stack = gp_vec_new(stack_size, sizeof(struct overlay_payload_elem));

	if (!overlay->stack) {
		free(ret);
		return NULL;
	}

	overlay->focused = -1;

	return ret;
}

unsigned int gp_widget_overlay_stack_size(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_OVERLAY, 0);
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	return gp_vec_len(overlay->stack);
}

int gp_widget_overlay_stack_pos_by_child(gp_widget *self, gp_widget *child,
                                         unsigned int *stack_pos)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_OVERLAY, 0);
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	unsigned int i;

	for (i = 0; i < gp_widget_overlay_stack_size(self); i++) {
		if (child == overlay->stack[i].widget) {
			*stack_pos = i;
			return 0;
		}
	}

	return 1;
}

static int stack_pos_is_invalid(gp_widget *self, unsigned int stack_pos)
{
	if (stack_pos >= gp_widget_overlay_stack_size(self)) {
		GP_WARN("Invalid stack_pos %u", stack_pos);
		return 1;
	}

	return 0;
}

void gp_widget_overlay_hide(gp_widget *self, unsigned int stack_pos)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_OVERLAY, );
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	if (stack_pos_is_invalid(self, stack_pos))
		return;

	if (overlay->stack[stack_pos].hidden)
		return;

	overlay->stack[stack_pos].hidden = 1;

	gp_widget_redraw_children(self);
}

void gp_widget_overlay_show(gp_widget *self, unsigned int stack_pos)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_OVERLAY, );
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);

	if (stack_pos_is_invalid(self, stack_pos))
		return;

	if (!overlay->stack[stack_pos].hidden)
		return;

	overlay->stack[stack_pos].hidden = 0;

	gp_widget_redraw_children(self);
}

gp_widget *gp_widget_overlay_put(gp_widget *self, unsigned int stack_pos,
                                 gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_OVERLAY, NULL);
	struct overlay_payload *overlay = GP_WIDGET_PAYLOAD(self);
	gp_widget *ret;

	if (stack_pos_is_invalid(self, stack_pos))
		return NULL;

	ret = overlay->stack[stack_pos].widget;
	overlay->stack[stack_pos].widget = child;

	gp_widget_set_parent(child, self);

	gp_widget_resize(self);

	return ret;
}
