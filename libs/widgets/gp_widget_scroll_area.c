//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_json.h>

static gp_size scroll_min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)ctx;
	//TODO: units!
	return self->scroll->min_w;
}

static gp_size scroll_min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)ctx;
	//TODO: units!
	return self->scroll->min_h;
}

static gp_size scrollbar_size(const gp_widget_render_ctx *ctx)
{
	return gp_text_ascent(ctx->font) + ctx->padd;
}

static unsigned int scrolls_x(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	if (!self->scroll->min_w)
		return 0;

	gp_size min_w = gp_widget_min_w(self->scroll->child, ctx);
	gp_size scroll_w = scroll_min_w(self, ctx);

	if (min_w > scroll_w) {
		self->scroll->scrollbar_y = 1;
		GP_DEBUG(4, "Scroll area %p scrolls horizontally", self);
		return 1;
	}

	self->scroll->scrollbar_y = 0;
	return 0;
}

static unsigned int scrolls_y(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	if (!self->scroll->min_h)
		return 0;

	gp_size min_h = gp_widget_min_h(self->scroll->child, ctx);
	gp_size scroll_h = scroll_min_h(self, ctx);

	if (min_h > scroll_h) {
		self->scroll->scrollbar_x = 1;
		GP_DEBUG(4, "Scroll area %p scrolls vertically", self);
		return 1;
	}

	self->scroll->scrollbar_x = 0;
	return 0;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	gp_size min_w;
	gp_size widget_min_w = gp_widget_min_w(self->scroll->child, ctx);

	if (!self->scroll->min_w)
		min_w = widget_min_w;
	else
		min_w = GP_MIN(scroll_min_w(self, ctx), widget_min_w);

	if (scrolls_y(self, ctx))
		min_w += scrollbar_size(ctx);

	return min_w;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	gp_size min_h;
	gp_size widget_min_h = gp_widget_min_w(self->scroll->child, ctx);

	if (!self->scroll->min_h)
		min_h = widget_min_h;
	else
		min_h = GP_MIN(scroll_min_h(self, ctx), widget_min_h);

	if (scrolls_x(self, ctx))
		min_h += scrollbar_size(ctx);

	return min_h;
}

static gp_size scrollbar_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_scroll_area *area = self->scroll;

	if (area->scrollbar_x && area->scrollbar_y)
		return self->w - scrollbar_size(ctx);

	return self->w;
}

static gp_size scrollbar_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_scroll_area *area = self->scroll;

	if (area->scrollbar_x && area->scrollbar_y)
		return self->h - scrollbar_size(ctx);

	return self->h;
}

static gp_coord max_x_off(gp_widget *self)
{
	if (self->w > self->scroll->child->w)
		return 0;

	return self->scroll->child->w - self->w;
}

static gp_coord max_y_off(gp_widget *self)
{
	if (self->h > self->scroll->child->h)
		return 0;

	return self->scroll->child->h - self->h;
}

static void draw_vert_scroll_bar(gp_widget *self, const gp_widget_render_ctx *ctx,
                                 gp_coord x, gp_coord y, gp_size h, gp_size size)
{
	struct gp_widget_scroll_area *area = self->scroll;
	gp_size asc = gp_text_ascent(ctx->font);

	gp_fill_rect_xywh(ctx->buf, x, y, size, h, ctx->bg_color);

	gp_size sh = scrollbar_h(self, ctx);

	gp_vline_xyh(ctx->buf, x + ctx->padd + asc/2, y, sh, ctx->text_color);

	gp_size max_off = max_y_off(self);
	gp_coord pos = ((sh - asc) * area->y_off + max_off/2) / max_off;

	gp_pixel col = area->area_focused ? ctx->sel_color : ctx->text_color;

	gp_fill_rrect_xywh(ctx->buf, x + ctx->padd, y + pos, asc, asc, ctx->bg_color, ctx->fg_color, col);
}

static void draw_horiz_scroll_bar(gp_widget *self, const gp_widget_render_ctx *ctx,
                                  gp_coord x, gp_coord y, gp_size w, gp_size size)
{
	struct gp_widget_scroll_area *area = self->scroll;
	gp_size asc = gp_text_ascent(ctx->font);

	gp_fill_rect_xywh(ctx->buf, x, y, w, size, ctx->bg_color);

	gp_size sw = scrollbar_w(self, ctx);

	gp_hline_xyw(ctx->buf, x, y + ctx->padd + asc/2, sw, ctx->text_color);

	gp_size max_off = max_x_off(self);
	gp_coord pos = ((sw - asc) * area->x_off + max_off/2) / max_off;

	gp_pixel col = area->area_focused ? ctx->sel_color : ctx->text_color;

	gp_fill_rrect_xywh(ctx->buf, x + pos, y + ctx->padd, asc, asc, ctx->bg_color, ctx->fg_color, col);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct gp_widget_scroll_area *area = self->scroll;
	gp_widget_render_ctx child_ctx = *ctx;
	gp_pixmap child_buf;
	gp_offset child_offset = {
		.x = -area->x_off,
		.y = -area->y_off,
	};

	//TODO!!!
	gp_widget_ops_blit(ctx, self->x + offset->x, self->y + offset->y, self->w, self->h);

	gp_size w = self->w;
	gp_size h = self->h;
	gp_size size = scrollbar_size(ctx);

	if (area->scrollbar_x) {
		w -= size;
		draw_vert_scroll_bar(self, ctx, self->x + offset->x + w, self->y + offset->y, h, size);
	}

	if (area->scrollbar_y) {
		h -= size;
		draw_horiz_scroll_bar(self, ctx, self->x + offset->x, self->y + offset->y + h, w, size);
	}

	gp_sub_pixmap(ctx->buf, &child_buf,
	              offset->x + self->x,
	              offset->y + self->y,
		      w, h);

	//TODO: Combine with passed down bbox?
	gp_bbox child_bbox = gp_bbox_pack(0, 0, w, h);

	child_ctx.bbox = &child_bbox;
	child_ctx.buf = &child_buf;
	//TODO: Propagate flip
	child_ctx.flip = NULL;

	gp_widget_ops_render(area->child, &child_offset, &child_ctx, flags);
	gp_rect_xywh(ctx->buf, self->x + offset->x, self->y + offset->y, w, h, ctx->text_color);
}

static int is_in_scrollbar_x(gp_widget *self, const gp_widget_render_ctx *ctx, unsigned int x)
{
	struct gp_widget_scroll_area *area = self->scroll;

	if (area->scrollbar_x) {
		if (x > self->x + self->w - scrollbar_size(ctx))
			return 1;
	}

	return 0;
}

static int is_in_scrollbar_y(gp_widget *self, const gp_widget_render_ctx *ctx, unsigned int y)
{
	struct gp_widget_scroll_area *area = self->scroll;

	if (area->scrollbar_y) {
		if (y > self->y + self->h - scrollbar_size(ctx))
			return 1;
	}

	return 0;
}

static void set_y_off(gp_widget *self, int y_off)
{
	if (y_off < 0) {
		GP_WARN("y_off < 0");
		return;
	}

	if (y_off > max_y_off(self)) {
		GP_WARN("y_off > max y_off");
		return;
	}

	if (self->scroll->y_off == y_off)
		return;

	self->scroll->y_off = y_off;

	gp_widget_redraw(self);
	gp_widget_redraw_children(self);
}

static void set_x_off(gp_widget *self, int x_off)
{
	if (x_off < 0) {
		GP_WARN("x_off < 0");
		return;
	}

	if (x_off > max_x_off(self)) {
		GP_WARN("y_off > max y_off");
		return;
	}

	if (self->scroll->x_off == x_off)
		return;

	self->scroll->x_off = x_off;

	gp_widget_redraw(self);
	gp_widget_redraw_children(self);
}

static void scrollbar_event_y(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	gp_size asc = gp_text_ascent(ctx->font);
	gp_size gh = scrollbar_h(self, ctx) - asc;
	gp_coord y = ev->st->cursor_y - asc/2;

	if (y < 0)
		y = 0;

	if ((gp_size)y > gh)
		y = gh;

	set_y_off(self, (y * max_y_off(self) + gh/2)/ gh);
}

static void scrollbar_event_x(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	gp_size asc = gp_text_ascent(ctx->font);
	gp_size gw = scrollbar_w(self, ctx) - asc;
	gp_coord x = ev->st->cursor_x - asc/2;

	if (x < 0)
		x = 0;

	if ((gp_size)x > gw)
		x = gw;

	set_x_off(self, (x * max_x_off(self) + gw/2)/ gw);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	struct gp_widget_scroll_area *area = self->scroll;

	if (is_in_scrollbar_y(self, ctx, ev->st->cursor_y)) {
		if (gp_event_key_pressed(ev, GP_BTN_LEFT) ||
		    ev->type == GP_EV_ABS) {
			scrollbar_event_x(self, ctx, ev);
			return 1;
		}
	}

	if (is_in_scrollbar_x(self, ctx, ev->st->cursor_x)) {
		if (gp_event_key_pressed(ev, GP_BTN_LEFT) ||
		    ev->type == GP_EV_ABS) {
			scrollbar_event_y(self, ctx, ev);
			return 1;
		}
	}

	if (area->area_focused) {
		if (ev->type != GP_EV_KEY)
			return 0;

		if (ev->code == GP_EV_KEY_UP)
			return 0;

		switch (ev->val) {
		case GP_KEY_LEFT:
			set_x_off(self, GP_MAX(0, area->x_off - 10));
		break;
		case GP_KEY_RIGHT:
			set_x_off(self, GP_MIN(max_x_off(self), area->x_off + 10));
		break;
		case GP_KEY_UP:
			set_y_off(self, GP_MAX(0, area->y_off - 10));
		break;
		case GP_KEY_DOWN:
			set_y_off(self, GP_MIN(max_y_off(self), area->y_off + 10));
		break;
		}

		return 0;
	}

	return gp_widget_ops_event_offset(area->child, ctx, ev, area->x_off, area->y_off);
}

static int focus_scrollbar(gp_widget *self)
{
	struct gp_widget_scroll_area *area = self->scroll;

	if (area->area_focused)
		return 1;

	area->area_focused = 1;
        gp_widget_ops_render_focus(area->child, GP_FOCUS_OUT);
	area->child_focused = 0;

	gp_widget_redraw(self);

	return 1;
}

static void focus_out(gp_widget *self)
{
	struct gp_widget_scroll_area *area = self->scroll;

	if (area->area_focused) {
		area->area_focused = 0;
		gp_widget_redraw(self);
	}
}

static int focus_widget(gp_widget *self, const gp_widget_render_ctx *ctx,
                         unsigned int x, unsigned int y)
{
	struct gp_widget_scroll_area *area = self->scroll;

	if (!gp_widget_ops_render_focus_xy(area->child, ctx, x + area->x_off, y + area->y_off))
		return 0;

	focus_out(self);

	area->child_focused = 1;
	return 1;
}

static int focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                     unsigned int x, unsigned int y)
{
	if (is_in_scrollbar_x(self, ctx, x) ||
	    is_in_scrollbar_y(self, ctx, y))
		return focus_scrollbar(self);

	return focus_widget(self, ctx, x, y);
}

static int focus(gp_widget *self, int sel)
{
	if (self->scroll->child_focused) {
		if (gp_widget_ops_render_focus(self->scroll->child, sel))
			return 1;
	}

	switch (sel) {
	case GP_FOCUS_OUT:
		focus_out(self);
	break;
	}

	return 0;
}

static int focus_child(gp_widget *self, gp_widget *child)
{
	if (self->scroll->child != child)
		return 0;

	focus_out(self);
	return 1;
}

static void distribute_size(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	struct gp_widget_scroll_area *area = self->scroll;

	gp_size child_min_w = gp_widget_min_w(area->child, ctx);
	gp_size child_min_h = gp_widget_min_h(area->child, ctx);
	gp_size w = self->w;
	gp_size h = self->h;

	if (area->scrollbar_x)
		w -= scrollbar_size(ctx);

	if (area->scrollbar_y)
		h -= scrollbar_size(ctx);

	gp_size child_w = GP_MAX(child_min_w, w);
	gp_size child_h = GP_MAX(child_min_h, h);

	gp_coord x_off = max_x_off(self);
	gp_coord y_off = max_y_off(self);

	if (area->x_off > x_off)
		area->x_off = x_off;

	if (x_off == 0)
		area->scrollbar_y = 0;
	else
		area->scrollbar_y = 1;

	if (area->y_off > y_off)
		area->y_off = y_off;

	if (y_off == 0)
		area->scrollbar_x = 0;
	else
		area->scrollbar_x = 1;

	gp_widget_ops_distribute_size(area->child, ctx, child_w, child_h, new_wh);
}

enum keys {
	MIN_H,
	MIN_W,
	WIDGET,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("min_h", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("min_w", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("widget", GP_JSON_OBJ),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_scroll(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	int min_w = 0;
	int min_h = 0;
	gp_widget *child = NULL, *ret;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case MIN_W:
			if (val->val_int < 0)
				gp_json_warn(json, "Size must be > 0!");
			else
				min_w = val->val_int;
		break;
		case MIN_H:
			if (val->val_int < 0)
				gp_json_warn(json, "Size must be > 0!");
			else
				min_h = val->val_int;
		break;
		case WIDGET:
			if (child)
				gp_json_err(json, "Duplicit widget key!");
			else
				child = gp_widget_from_json(json, val, ctx);
		break;
		}
	}

	if (min_w == 0 && min_h == 0) {
		gp_json_warn(json, "At least one of min_w and min_h must be > 0");
		gp_widget_free(child);
		return NULL;
	}

	ret = gp_widget_scroll_area_new(min_w, min_h, child);
	if (!ret)
		gp_widget_free(child);

	return ret;
}

struct gp_widget_ops gp_widget_scroll_area_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.focus_xy = focus_xy,
	.focus = focus,
	.focus_child = focus_child,
	.distribute_size = distribute_size,
	.from_json = json_to_scroll,
	.id = "scroll area",
};

gp_widget *gp_widget_scroll_area_new(gp_size min_w, gp_size min_h, gp_widget *child)
{
	gp_widget *ret;

	ret = gp_widget_new(GP_WIDGET_SCROLL_AREA, GP_WIDGET_CLASS_NONE, sizeof(struct gp_widget_scroll_area));
	if (!ret)
		return NULL;

	ret->scroll->min_w = min_w;
	ret->scroll->min_h = min_h;

	ret->scroll->child = child;
	gp_widget_set_parent(child, ret);

	return ret;
}

static int move_x(gp_widget *self, gp_coord x_off)
{
	struct gp_widget_scroll_area *area = self->scroll;

	gp_coord old_x_off = area->x_off;

	area->x_off += x_off;

	if (area->x_off < 0)
		area->x_off = 0;

	if (area->x_off + self->w > area->child->w)
		area->x_off = area->child->w - self->w;

	if (area->x_off == old_x_off)
		return 0;

	return 1;
}

static int move_y(gp_widget *self, gp_coord y_off)
{
	struct gp_widget_scroll_area *area = self->scroll;

	gp_coord old_y_off = area->y_off;

	area->y_off += y_off;

	if (area->y_off < 0)
		area->y_off = 0;

	if (area->y_off + self->h > area->child->h)
		area->y_off = area->child->h - self->h;

	if (area->y_off == old_y_off)
		return 0;

	return 1;
}

int gp_widget_scroll_area_move(gp_widget *self, gp_coord x_off, gp_coord y_off)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_SCROLL_AREA, 1);

	struct gp_widget_scroll_area *area = self->scroll;

	if (!area->child)
		return 1;

	int ret = 0;

	if (area->scrollbar_y)
		ret |= move_x(self, x_off);

	if (area->scrollbar_x)
		ret |= move_y(self, y_off);

	if (!ret)
		return ret;

	gp_widget_redraw(self);
	gp_widget_redraw_children(self);

	return 1;
}

gp_widget *gp_widget_scroll_area_put(gp_widget *self, gp_widget *child)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_SCROLL_AREA, NULL);

	gp_widget *ret = self->scroll->child;

	self->scroll->child = child;
	gp_widget_set_parent(child, self);

	gp_widget_resize(self);

	return ret;
}
