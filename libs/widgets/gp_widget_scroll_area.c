//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_json.h>

struct scroll_area_payload {
	/* offset for the layout inside */
	gp_coord x_off;
	gp_coord y_off;

	/*
	 * If non-zero the widget minimal size is set into the stone
	 * and the content scrolls if the inner widget size is bigger.
	 */
	gp_size min_w;
	gp_size min_h;

	int scrollbar_x:1;
	int scrollbar_y:1;
	int area_focused:1;
	int child_focused:1;

	gp_widget *child;
};

static gp_size scroll_min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);
	(void)ctx;
	//TODO: units!
	return scroll->min_w;
}

static gp_size scroll_min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);
	(void)ctx;
	//TODO: units!
	return scroll->min_h;
}

static gp_size scrollbar_size(const gp_widget_render_ctx *ctx)
{
	return gp_text_ascent(ctx->font) + ctx->padd;
}

static unsigned int scrolls_x(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (!scroll->min_w)
		return 0;

	gp_size min_w = gp_widget_min_w(scroll->child, ctx);
	gp_size scroll_w = scroll_min_w(self, ctx);

	if (min_w > scroll_w) {
		scroll->scrollbar_y = 1;
		GP_DEBUG(4, "Scroll area %p scrolls horizontally", self);
		return 1;
	}

	scroll->scrollbar_y = 0;
	return 0;
}

static unsigned int scrolls_y(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (!scroll->min_h)
		return 0;

	gp_size min_h = gp_widget_min_h(scroll->child, ctx);
	gp_size scroll_h = scroll_min_h(self, ctx);

	if (min_h > scroll_h) {
		scroll->scrollbar_x = 1;
		GP_DEBUG(4, "Scroll area %p scrolls vertically", self);
		return 1;
	}

	scroll->scrollbar_x = 0;
	return 0;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	gp_size min_w;
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);
	gp_size widget_min_w = gp_widget_min_w(scroll->child, ctx);

	if (!scroll->min_w)
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
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);
	gp_size widget_min_h = gp_widget_min_w(scroll->child, ctx);

	if (!scroll->min_h)
		min_h = widget_min_h;
	else
		min_h = GP_MIN(scroll_min_h(self, ctx), widget_min_h);

	if (scrolls_x(self, ctx))
		min_h += scrollbar_size(ctx);

	return min_h;
}

static gp_size scrollbar_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (scroll->scrollbar_x && scroll->scrollbar_y)
		return self->w - scrollbar_size(ctx);

	return self->w;
}

static gp_size scrollbar_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (scroll->scrollbar_x && scroll->scrollbar_y)
		return self->h - scrollbar_size(ctx);

	return self->h;
}

static gp_coord max_x_off(gp_widget *self)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (self->w > scroll->child->w)
		return 0;

	return scroll->child->w - self->w;
}

static gp_coord max_y_off(gp_widget *self)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (self->h > scroll->child->h)
		return 0;

	return scroll->child->h - self->h;
}

static void draw_vert_scroll_bar(gp_widget *self, const gp_widget_render_ctx *ctx,
                                 gp_coord x, gp_coord y, gp_size h, gp_size size, gp_pixel color)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);
	gp_size asc = gp_text_ascent(ctx->font);

	gp_fill_rect_xywh(ctx->buf, x, y, size, h, ctx->bg_color);

	gp_size sh = scrollbar_h(self, ctx);

	gp_vline_xyh(ctx->buf, x + ctx->padd + asc/2, y, sh, color);

	gp_size max_off = max_y_off(self);
	gp_coord pos = ((sh - asc) * scroll->y_off + max_off/2) / max_off;

	gp_pixel col = scroll->area_focused ? ctx->sel_color : color;

	gp_fill_rrect_xywh(ctx->buf, x + ctx->padd, y + pos, asc, asc, ctx->bg_color, ctx->fg_color, col);
}

static void draw_horiz_scroll_bar(gp_widget *self, const gp_widget_render_ctx *ctx,
                                  gp_coord x, gp_coord y, gp_size w, gp_size size, gp_pixel color)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);
	gp_size asc = gp_text_ascent(ctx->font);

	gp_fill_rect_xywh(ctx->buf, x, y, w, size, ctx->bg_color);

	gp_size sw = scrollbar_w(self, ctx);

	gp_hline_xyw(ctx->buf, x, y + ctx->padd + asc/2, sw, color);

	gp_size max_off = max_x_off(self);
	gp_coord pos = ((sw - asc) * scroll->x_off + max_off/2) / max_off;

	gp_pixel col = scroll->area_focused ? ctx->sel_color : color;

	gp_fill_rrect_xywh(ctx->buf, x + pos, y + ctx->padd, asc, asc, ctx->bg_color, ctx->fg_color, col);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);
	gp_widget_render_ctx child_ctx = *ctx;
	gp_pixmap child_buf;
	gp_offset child_offset = {
		.x = -scroll->x_off,
		.y = -scroll->y_off,
	};
	gp_pixel text_color = ctx->text_color;

	if (gp_widget_is_disabled(self, flags))
		text_color = ctx->col_disabled;

	//TODO!!!
	gp_widget_ops_blit(ctx, self->x + offset->x, self->y + offset->y, self->w, self->h);

	gp_size w = self->w;
	gp_size h = self->h;
	gp_size size = scrollbar_size(ctx);

	if (scroll->scrollbar_x) {
		w -= size;
		draw_vert_scroll_bar(self, ctx, self->x + offset->x + w, self->y + offset->y, h, size, text_color);
	}

	if (scroll->scrollbar_y) {
		h -= size;
		draw_horiz_scroll_bar(self, ctx, self->x + offset->x, self->y + offset->y + h, w, size, text_color);
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

	gp_widget_ops_render(scroll->child, &child_offset, &child_ctx, flags);
	gp_rect_xywh(ctx->buf, self->x + offset->x, self->y + offset->y, w, h, text_color);
}

static int is_in_scrollbar_x(gp_widget *self, const gp_widget_render_ctx *ctx, unsigned int x)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (scroll->scrollbar_x) {
		if (x > self->x + self->w - scrollbar_size(ctx))
			return 1;
	}

	return 0;
}

static int is_in_scrollbar_y(gp_widget *self, const gp_widget_render_ctx *ctx, unsigned int y)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (scroll->scrollbar_y) {
		if (y > self->y + self->h - scrollbar_size(ctx))
			return 1;
	}

	return 0;
}

static void set_y_off(gp_widget *self, int y_off)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (y_off < 0) {
		GP_WARN("y_off < 0");
		return;
	}

	if (y_off > max_y_off(self)) {
		GP_WARN("y_off > max y_off");
		return;
	}

	if (scroll->y_off == y_off)
		return;

	scroll->y_off = y_off;

	gp_widget_redraw(self);
	gp_widget_redraw_children(self);
}

static void set_x_off(gp_widget *self, int x_off)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (x_off < 0) {
		GP_WARN("x_off < 0");
		return;
	}

	if (x_off > max_x_off(self)) {
		GP_WARN("y_off > max y_off");
		return;
	}

	if (scroll->x_off == x_off)
		return;

	scroll->x_off = x_off;

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
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (is_in_scrollbar_y(self, ctx, ev->st->cursor_y)) {
		if (gp_ev_key_pressed(ev, GP_BTN_LEFT) ||
		    ev->type == GP_EV_ABS) {
			scrollbar_event_x(self, ctx, ev);
			return 1;
		}
	}

	if (is_in_scrollbar_x(self, ctx, ev->st->cursor_x)) {
		if (gp_ev_key_pressed(ev, GP_BTN_LEFT) ||
		    ev->type == GP_EV_ABS) {
			scrollbar_event_y(self, ctx, ev);
			return 1;
		}
	}

	if (scroll->area_focused) {
		if (ev->type != GP_EV_KEY)
			return 0;

		if (ev->code == GP_EV_KEY_UP)
			return 0;

		switch (ev->val) {
		case GP_KEY_LEFT:
			set_x_off(self, GP_MAX(0, scroll->x_off - 10));
		break;
		case GP_KEY_RIGHT:
			set_x_off(self, GP_MIN(max_x_off(self), scroll->x_off + 10));
		break;
		case GP_KEY_UP:
			set_y_off(self, GP_MAX(0, scroll->y_off - 10));
		break;
		case GP_KEY_DOWN:
			set_y_off(self, GP_MIN(max_y_off(self), scroll->y_off + 10));
		break;
		}

		return 0;
	}

	return gp_widget_ops_event_offset(scroll->child, ctx, ev, scroll->x_off, scroll->y_off);
}

static int focus_scrollbar(gp_widget *self)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (scroll->area_focused)
		return 1;

	scroll->area_focused = 1;
        gp_widget_ops_render_focus(scroll->child, GP_FOCUS_OUT);
	scroll->child_focused = 0;

	gp_widget_redraw(self);

	return 1;
}

static void focus_out(gp_widget *self)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (scroll->area_focused) {
		scroll->area_focused = 0;
		gp_widget_redraw(self);
	}
}

static int focus_widget(gp_widget *self, const gp_widget_render_ctx *ctx,
                         unsigned int x, unsigned int y)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (!gp_widget_ops_render_focus_xy(scroll->child, ctx, x + scroll->x_off, y + scroll->y_off))
		return 0;

	focus_out(self);

	scroll->child_focused = 1;
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
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (scroll->child_focused) {
		if (gp_widget_ops_render_focus(scroll->child, sel))
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
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (scroll->child != child)
		return 0;

	focus_out(self);
	return 1;
}

static void distribute_w(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	gp_size child_min_w = gp_widget_min_w(scroll->child, ctx);
	gp_size w = self->w;

	if (scroll->scrollbar_x)
		w -= scrollbar_size(ctx);

	gp_size child_w = GP_MAX(child_min_w, w);

	gp_coord x_off = max_x_off(self);

	if (scroll->x_off > x_off)
		scroll->x_off = x_off;

	if (x_off == 0)
		scroll->scrollbar_y = 0;
	else
		scroll->scrollbar_y = 1;

	gp_widget_ops_distribute_w(scroll->child, ctx, child_w, new_wh);
}

static void distribute_h(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	gp_size child_min_h = gp_widget_min_h(scroll->child, ctx);
	gp_size h = self->h;

	if (scroll->scrollbar_y)
		h -= scrollbar_size(ctx);

	gp_size child_h = GP_MAX(child_min_h, h);

	gp_coord y_off = max_y_off(self);

	if (scroll->y_off > y_off)
		scroll->y_off = y_off;

	if (y_off == 0)
		scroll->scrollbar_x = 0;
	else
		scroll->scrollbar_x = 1;

	gp_widget_ops_distribute_h(scroll->child, ctx, child_h, new_wh);
}

enum keys {
	MIN_H,
	MIN_W,
	WIDGET,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(MIN_H, "min_h", GP_JSON_INT),
	GP_JSON_OBJ_ATTR_IDX(MIN_W, "min_w", GP_JSON_INT),
	GP_JSON_OBJ_ATTR_IDX(WIDGET, "widget", GP_JSON_OBJ),
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

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
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
	.distribute_w = distribute_w,
	.distribute_h = distribute_h,
	.from_json = json_to_scroll,
	.id = "scroll area",
};

gp_widget *gp_widget_scroll_area_new(gp_size min_w, gp_size min_h, gp_widget *child)
{
	gp_widget *ret;

	ret = gp_widget_new(GP_WIDGET_SCROLL_AREA, GP_WIDGET_CLASS_NONE, sizeof(struct scroll_area_payload));
	if (!ret)
		return NULL;

	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(ret);

	scroll->min_w = min_w;
	scroll->min_h = min_h;

	scroll->child = child;
	gp_widget_set_parent(child, ret);

	return ret;
}

static int move_x(gp_widget *self, gp_coord x_off)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	gp_coord old_x_off = scroll->x_off;

	scroll->x_off += x_off;

	if (scroll->x_off < 0)
		scroll->x_off = 0;

	if (scroll->x_off + self->w > scroll->child->w)
		scroll->x_off = scroll->child->w - self->w;

	if (scroll->x_off == old_x_off)
		return 0;

	return 1;
}

static int move_y(gp_widget *self, gp_coord y_off)
{
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	gp_coord old_y_off = scroll->y_off;

	scroll->y_off += y_off;

	if (scroll->y_off < 0)
		scroll->y_off = 0;

	if (scroll->y_off + self->h > scroll->child->h)
		scroll->y_off = scroll->child->h - self->h;

	if (scroll->y_off == old_y_off)
		return 0;

	return 1;
}

int gp_widget_scroll_area_move(gp_widget *self, gp_coord x_off, gp_coord y_off)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_SCROLL_AREA, 1);

	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	if (!scroll->child)
		return 1;

	int ret = 0;

	if (scroll->scrollbar_y)
		ret |= move_x(self, x_off);

	if (scroll->scrollbar_x)
		ret |= move_y(self, y_off);

	if (!ret)
		return ret;

	gp_widget_redraw(self);
	gp_widget_redraw_children(self);

	return 1;
}

gp_widget *gp_widget_scroll_area_put(gp_widget *self, gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_SCROLL_AREA, NULL);
	struct scroll_area_payload *scroll = GP_WIDGET_PAYLOAD(self);

	gp_widget *ret = scroll->child;

	scroll->child = child;
	gp_widget_set_parent(child, self);

	gp_widget_resize(self);

	return ret;
}
