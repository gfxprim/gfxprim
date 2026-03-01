//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2026 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <utils/gp_vec_str.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

struct activity_payload {
	char *label;
	uint8_t active;
	int8_t dir;
	uint8_t running;
};

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct activity_payload *activity = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = ctx->font;
	unsigned int max_width;

	max_width = 5 * ctx->padd + 5 * (2*gp_text_ascent(font)/3);

	if (activity->label)
		max_width = GP_MAX(max_width, gp_text_wbbox(font, activity->label));

	return max_width + 2 * ctx->padd;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct activity_payload *activity = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = ctx->font;
	unsigned int h = 0;

	if (activity->label)
		h = gp_text_ascent(font) + ctx->padd;

	return h + 2 * ctx->padd + gp_text_ascent(font);
}

static void activity_elem(gp_pixmap *pixmap, const gp_widget_render_ctx *ctx, gp_coord cx, gp_coord cy, gp_size r, gp_pixel col_fg, gp_pixel col_fr)
{
	if (r <= 2) {
		gp_fill_circle(pixmap, cx, cy, r, col_fg);
		gp_circle(pixmap, cx, cy, r, col_fr);
	} else {
		gp_fill_rrect_xywh(pixmap, cx - r, cy - r, 2*r, 2*r, ctx->bg_color, col_fg, col_fr);
	}
}

static void start_timer(gp_widget *self)
{
	struct activity_payload *activity = GP_WIDGET_PAYLOAD(self);
	int sleeps[5] = {1000, 200, 100, 200, 1000};

	gp_widget_render_timer(self, 0, sleeps[activity->active]);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct activity_payload *activity = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = ctx->font;

	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	gp_pixel text_color = ctx->text_color;
	gp_pixel bg = ctx->bg_color;
	gp_pixel fg = ctx->fg_color;

	(void) flags;

	if (gp_widget_is_disabled(self, flags))
		text_color = ctx->col_disabled;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_fill_rect_xywh(ctx->buf, x, y, w, h, bg);

	if (activity->label) {
		gp_text(ctx->buf, font,
			x+w/2, y+ctx->padd, GP_ALIGN_CENTER|GP_VALIGN_BELOW,
			text_color, ctx->bg_color, activity->label);
		y += 2 * ctx->padd + gp_text_ascent(font);
	}

	unsigned int a = gp_text_ascent(font);
	unsigned int cx = x+w/2;
	unsigned int cy = y+a/2;
	unsigned int dx = 2*(a/3) + ctx->padd;

	unsigned int r = a/3;
	gp_pixel fgs[5] = {fg, fg, fg, fg, fg};
	unsigned int cys[5] = {cy, cy, cy, cy, cy};
	unsigned int act = activity->active%5;

	if (r <= 1)
		r = 2;

	fgs[act] = ctx->hl_color;

	int add = GP_MAX(1, ctx->padd/4) * activity->dir;

	if (act > 0 && act < 4) {
		cys[act] -= add;
		cys[act-1] += add;
		cys[act+1] += add;
	}

	if (act > 1 && act < 3) {
		cys[act] -= add;
		cys[act-1] += add;
		cys[act+1] += add;
		cys[act-2] -= add;
		cys[act+2] -= add;
	}

	activity_elem(ctx->buf, ctx, cx-2*dx, cys[0], r, fgs[0], text_color);
	activity_elem(ctx->buf, ctx, cx-dx, cys[1], r, fgs[1], text_color);
	activity_elem(ctx->buf, ctx, cx, cys[2], r, fgs[2], text_color);
	activity_elem(ctx->buf, ctx, cx+dx, cys[3], r, fgs[3], text_color);
	activity_elem(ctx->buf, ctx, cx+2*dx, cys[4], r, fgs[4], text_color);
}

enum keys {
	LABEL,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(LABEL, "label", GP_JSON_STR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_activity(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	char *label = NULL;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case LABEL:
			label = strdup(val->val_str);
		break;
		}
	}

	gp_widget *ret = gp_widget_activity_new(label);

	free(label);

	return ret;
}

static void free_(gp_widget *self)
{
	struct activity_payload *activity = GP_WIDGET_PAYLOAD(self);

	free(activity->label);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	struct activity_payload *activity = GP_WIDGET_PAYLOAD(self);
	(void) ctx;

	switch (ev->type) {
	case GP_EV_TMR:
		activity->active += activity->dir;

		if (activity->active == 0)
			activity->dir = 1;

		if (activity->active == 4)
			activity->dir = -1;

		gp_widget_redraw(self);

		start_timer(self);

		return 1;
	break;
	}

	return 0;
}

struct gp_widget_ops gp_widget_activity_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.free = free_,
	.from_json = json_to_activity,
	.id = "activity",
};

gp_widget *gp_widget_activity_new(const char *label)
{
	gp_widget *ret;

	ret = gp_widget_new(GP_WIDGET_ACTIVITY, GP_WIDGET_CLASS_NONE, sizeof(struct activity_payload));
	if (!ret)
		return NULL;

	struct activity_payload *activity = GP_WIDGET_PAYLOAD(ret);
	if (label)
		activity->label = strdup(label);

	activity->active = 0;
	activity->dir = 1;

	return ret;
}

void gp_widget_activity_start(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_ACTIVITY, );
	struct activity_payload *activity = GP_WIDGET_PAYLOAD(self);

	if (activity->running)
		return;

	activity->running = 1;

	start_timer(self);
}

void gp_widget_activity_stop(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_ACTIVITY, );
	struct activity_payload *activity = GP_WIDGET_PAYLOAD(self);

	if (!activity->running)
		return;

	activity->running = 0;

	gp_widget_render_timer_cancel(self);

	activity->active = 0;
	activity->dir = 1;

	gp_widget_redraw(self);
}
