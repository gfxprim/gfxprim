//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <core/gp_common.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_json.h>

struct gp_widget_frame {
	gp_widget *child;
	gp_widget_tattr tattr;
	int light_bg:1;
	char *title;
};

static unsigned int frame_w(const gp_widget_render_ctx *ctx)
{
	return 2 * ctx->padd;
}

static unsigned int frame_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(frame->tattr, ctx);

	if (frame->title)
		return ctx->padd + gp_text_height(font);

	return 2 * ctx->padd;
}

static unsigned int payload_off_x(const gp_widget_render_ctx *ctx)
{
	return ctx->padd;
}

static unsigned int payload_off_y(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(frame->tattr, ctx);

	if (frame->title)
		return gp_text_height(font);

	return ctx->padd;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(frame->tattr, ctx);
	unsigned int min_w;

	min_w = GP_MAX(gp_text_wbbox(font, frame->title) + 2 * ctx->padd,
		       gp_widget_min_w(frame->child, ctx));

	return frame_w(ctx) + min_w;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);

	return frame_h(self, ctx) + gp_widget_min_h(frame->child, ctx);
}

static void distribute_w(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);
	unsigned int w = self->w - frame_w(ctx);

	if (frame->child)
		gp_widget_ops_distribute_w(frame->child, ctx, w, new_wh);
}

static void distribute_h(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);
	unsigned int h = self->h - frame_h(self, ctx);

	if (frame->child)
		gp_widget_ops_distribute_h(frame->child, ctx, h, new_wh);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	struct gp_widget *payload = frame->child;
	const gp_text_style *font = gp_widget_tattr_font(frame->tattr, ctx);
	gp_pixel text_color = ctx->text_color;

	if (gp_widget_is_disabled(self, flags))
		text_color = ctx->col_disabled;

	if (gp_widget_should_redraw(self, flags)) {
		gp_widget_ops_blit(ctx, x, y, w, h);

		gp_pixel fg_color = frame->light_bg ? ctx->fg_color : ctx->bg_color;

		gp_fill_rect_xywh(ctx->buf, x, y, w, gp_text_height(font)/2, ctx->bg_color);

		gp_fill_rrect_xywh(ctx->buf, x, y + payload_off_y(self, ctx)/2, w,
		              h - payload_off_y(self, ctx)/2, ctx->bg_color, fg_color, text_color);

		if (frame->title) {
			unsigned int sw = gp_text_wbbox(font, frame->title) + ctx->padd;

			gp_fill_rect_xywh(ctx->buf, x + ctx->padd + ctx->padd/2, y,
			                  sw, gp_text_height(font), fg_color);

			gp_text(ctx->buf, font, x + 2 * ctx->padd, y, GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
				text_color, ctx->bg_color, frame->title);
		}
	}

	if (!payload)
		return;

	gp_offset widget_offset = {
		.x = x + payload_off_x(ctx),
		.y = y + payload_off_y(self, ctx),
	};

	gp_widget_render_ctx ctx_cpy = *ctx;

	if (frame->light_bg)
		GP_SWAP(ctx_cpy.fg_color, ctx_cpy.bg_color);

	gp_widget_ops_render(frame->child, &widget_offset, &ctx_cpy, flags);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);
	unsigned int px = payload_off_x(ctx);
	unsigned int py = payload_off_y(self, ctx);

	return gp_widget_ops_event_offset(frame->child, ctx, ev, px, py);
}

static int focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                     unsigned int x, unsigned int y)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);

	return gp_widget_ops_render_focus_xy(frame->child, ctx,
	                                     x - payload_off_x(ctx),
	                                     y - payload_off_y(self, ctx));
}

static int focus(gp_widget *self, int sel)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);

	return gp_widget_ops_render_focus(frame->child, sel);
}

enum keys {
	BG,
	TATTR,
	TITLE,
	WIDGET,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(BG, "bg", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(TATTR, "tattr", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(TITLE, "title", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(WIDGET, "widget", GP_JSON_OBJ),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_frame(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	char *title = NULL;
	gp_widget_tattr tattr = GP_TATTR_BOLD;
	gp_widget *child = NULL;
	int light_bg = 0;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case BG:
			if (!strcmp(val->val_str, "light"))
				light_bg = 1;
			else if (!strcmp(val->val_str, "dark"))
				light_bg = 0;
			else
				gp_json_warn(json, "Invalid bg attribute");
		break;
		case TATTR:
			if (gp_widget_tattr_parse(val->val_str, &tattr, GP_TATTR_FONT))
				gp_json_warn(json, "Invalid text attribute");
		break;
		case TITLE:
			title = strdup(val->val_str);
		break;
		case WIDGET:
			child = gp_widget_from_json(json, val, ctx);
		break;
		}
	}

	gp_widget *ret = gp_widget_frame_new(NULL, tattr, child);
	if (!ret) {
		gp_widget_free(child);
		return NULL;
	}

	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(ret);

	frame->title = title;
	frame->light_bg = light_bg;

	return ret;
}

static void frame_free(gp_widget *self)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);

	free(frame->title);
}

static int focus_child(gp_widget *self, gp_widget *child)
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);

	return frame->child == child;
}

static void for_each_child(gp_widget *self, void (*func)(gp_widget *child))
{
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);

	gp_widget *child = frame->child;

	if (child)
		func(child);
}

struct gp_widget_ops gp_widget_frame_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.free = frame_free,
	.focus_xy = focus_xy,
	.focus = focus,
	.focus_child = focus_child,
	.for_each_child = for_each_child,
	.distribute_w = distribute_w,
	.distribute_h = distribute_h,
	.from_json = json_to_frame,
	.id = "frame",
};

gp_widget *gp_widget_frame_new(const char *title, gp_widget_tattr tattr,
                               gp_widget *child)
{
	gp_widget *ret;
	size_t size = sizeof(struct gp_widget_frame);

	ret = gp_widget_new(GP_WIDGET_FRAME, GP_WIDGET_CLASS_NONE, size);
	if (!ret)
		return NULL;

	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(ret);

	frame->child = child;
	frame->tattr = tattr;

	if (title)
		frame->title = strdup(title);

	gp_widget_set_parent(child, ret);

	return ret;
}

gp_widget *gp_widget_frame_put(gp_widget *self, gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_FRAME, NULL);
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);
	gp_widget *ret;

	ret = frame->child;
	frame->child = child;

	gp_widget_set_parent(child, self);

	gp_widget_resize(self);

	return ret;
}

void gp_widget_frame_title_set(gp_widget *self, const char *title)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_FRAME, );
	struct gp_widget_frame *frame = GP_WIDGET_PAYLOAD(self);
	char *dup = NULL;

	if (title) {
		dup = strdup(title);
		if (!dup)
			return;
	}

	free(frame->title);
	frame->title = dup;

	gp_widget_resize(self);
	gp_widget_redraw(self);
}
