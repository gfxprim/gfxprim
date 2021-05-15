//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <core/gp_common.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_json.h>

static unsigned int frame_w(const gp_widget_render_ctx *ctx)
{
	return 2 * ctx->padd;
}

static unsigned int frame_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	const gp_text_style *font = gp_widget_tattr_font(self->frame->tattr, ctx);

	if (self->frame->title) {
		return ctx->padd + gp_text_height(font);
	}

	return 2 * ctx->padd;
}

static unsigned int payload_off_x(const gp_widget_render_ctx *ctx)
{
	return ctx->padd;
}

static unsigned int payload_off_y(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	const gp_text_style *font = gp_widget_tattr_font(self->frame->tattr, ctx);

	if (self->frame->title)
		return gp_text_height(font);

	return ctx->padd;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	const gp_text_style *font = gp_widget_tattr_font(self->frame->tattr, ctx);
	unsigned int min_w;

	min_w = GP_MAX(gp_text_width(font, self->frame->title) + 2 * ctx->padd,
		       gp_widget_min_w(self->frame->child, ctx));

	return frame_w(ctx) + min_w;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	return frame_h(self, ctx) + gp_widget_min_h(self->frame->child, ctx);
}

static void distribute_size(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	unsigned int w = self->w - frame_w(ctx);
	unsigned int h = self->h - frame_h(self, ctx);

	if (self->frame->child)
		gp_widget_ops_distribute_size(self->frame->child, ctx, w, h, new_wh);
}

static void draw_bg_around_payload(gp_widget *self, const gp_widget_render_ctx *ctx,
                                   gp_coord x, gp_coord y, gp_size w, gp_size h)
{
	gp_widget *payload = self->frame->child;

	gp_fill_rect_xywh(ctx->buf, x, y, w, payload_off_y(self, ctx), ctx->bg_color);
	gp_fill_rect_xywh(ctx->buf, x, y+h-ctx->padd, w, ctx->padd, ctx->bg_color);

	unsigned int padd_w = ctx->padd + payload->x;

	gp_fill_rect_xywh(ctx->buf, x, y + payload_off_y(self, ctx), padd_w,
			  h-ctx->padd - payload_off_y(self, ctx), ctx->bg_color);

	unsigned int padd_x = x + padd_w;
	padd_w = ctx->padd;

	padd_x += payload->w;
	padd_w += w - payload->x - payload->w - 2 * ctx->padd;

	gp_fill_rect_xywh(ctx->buf, padd_x, y + payload_off_y(self, ctx),
	                  padd_w, h - ctx->padd - payload_off_y(self, ctx), ctx->bg_color);

	gp_fill_rect_xywh(ctx->buf,
	                  x + payload->x + payload_off_x(ctx),
			  y + payload_off_y(self, ctx),
			  payload->w,
			  payload->y, ctx->bg_color);

	gp_fill_rect_xywh(ctx->buf,
	                  x + payload->x + payload_off_x(ctx),
	                  y + payload_off_y(self, ctx) + payload->y,
			  payload->w,
			  h - payload_off_y(self, ctx) - payload->y - ctx->padd, ctx->bg_color);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	struct gp_widget_frame *frame = self->frame;
	struct gp_widget *payload = frame->child;
	const gp_text_style *font = gp_widget_tattr_font(self->frame->tattr, ctx);

	if (gp_widget_should_redraw(self, flags)) {
		gp_widget_ops_blit(ctx, x, y, w, h);

		if (payload)
			draw_bg_around_payload(self, ctx, x, y, w, h);
		else
			gp_fill_rect_xywh(ctx->buf, x, y, w, h, ctx->bg_color);

		gp_rrect_xywh(ctx->buf, x + ctx->padd/2, y + payload_off_y(self, ctx)/2, w - ctx->padd,
		              h - ctx->padd/2 - payload_off_y(self, ctx)/2, ctx->text_color);

		if (frame->title) {
			unsigned int sw = gp_text_width(font, self->frame->title) + ctx->padd;

			gp_fill_rect_xywh(ctx->buf, x + ctx->padd + ctx->padd/2, y,
			                  sw, gp_text_height(font), ctx->bg_color);

			gp_text(ctx->buf, font, x + 2 * ctx->padd, y, GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
				ctx->text_color, ctx->bg_color, self->frame->title);
		}
	}

	if (!payload)
		return;

	gp_offset widget_offset = {
		.x = x + payload_off_x(ctx),
		.y = y + payload_off_y(self, ctx),
	};

	gp_widget_ops_render(self->frame->child, &widget_offset, ctx, flags);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	unsigned int px = payload_off_x(ctx);
	unsigned int py = payload_off_y(self, ctx);

	return gp_widget_ops_event_offset(self->frame->child, ctx, ev, px, py);
}

static int focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                     unsigned int x, unsigned int y)
{
	return gp_widget_ops_render_focus_xy(self->frame->child, ctx,
	                                     x - payload_off_x(ctx),
	                                     y - payload_off_y(self, ctx));
}

static int focus(gp_widget *self, int sel)
{
	return gp_widget_ops_render_focus(self->frame->child, sel);
}

static gp_widget *json_to_frame(json_object *json, gp_htable **uids)
{
	const char *title = NULL;
	json_object *json_child = NULL;
	const char *strattr = NULL;
	gp_widget_tattr tattr = GP_TATTR_BOLD;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "title"))
			title = json_object_get_string(val);
		else if (!strcmp(key, "widget"))
			json_child = val;
		else if (!strcmp(key, "tattr"))
			strattr = json_object_get_string(val);
		else
			GP_WARN("Invalid frame key '%s'", key);
	}

	if (strattr && gp_widget_tattr_parse(strattr, &tattr))
		GP_WARN("Invalid text attribute '%s'", strattr);

	gp_widget *child = gp_widget_from_json(json_child, uids);
	gp_widget *ret = gp_widget_frame_new(title, tattr, child);

	if (!ret)
		gp_widget_free(child);

	return ret;
}

static void frame_free(gp_widget *self)
{
	free(self->frame->title);
}

struct gp_widget_ops gp_widget_frame_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.free = frame_free,
	.focus_xy = focus_xy,
	.focus = focus,
	.distribute_size = distribute_size,
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

	ret->frame->child = child;
	ret->frame->tattr = tattr;

	if (title)
		ret->frame->title = strdup(title);

	gp_widget_set_parent(child, ret);

	return ret;
}

gp_widget *gp_widget_frame_put(gp_widget *self, gp_widget *child)
{
	gp_widget *ret;

	GP_WIDGET_ASSERT(self, GP_WIDGET_FRAME, NULL);

	ret = self->frame->child;
	self->frame->child = child;

	gp_widget_set_parent(child, self);

	gp_widget_resize(self);

	return ret;
}

void gp_widget_frame_title_set(gp_widget *self, const char *title)
{
	char *dup = NULL;

	GP_WIDGET_ASSERT(self, GP_WIDGET_FRAME, );

	if (title) {
		dup = strdup(title);
		if (!dup)
			return;
	}

	free(self->frame->title);
	self->frame->title = dup;

	gp_widget_resize(self);
	gp_widget_redraw(self);
}
