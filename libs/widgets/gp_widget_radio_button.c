//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_string.h>
#include "gp_widget_choice_priv.h"

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	const gp_text_style *font = gp_widget_focused_font(ctx, 1);
	unsigned int text_a = gp_text_ascent(font);
	unsigned int text_w = 0;
	unsigned int i, w;

	for (i = 0; i < gp_widget_choice_cnt_get(self); i++) {
		w = gp_text_wbbox(font, call_get_choice(self, i));
		text_w = GP_MAX(text_w, w);
	}

	return ctx->padd + text_a + text_w;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	return ctx->padd +
	       gp_widget_choice_cnt_get(self) * (gp_text_ascent(ctx->font) + ctx->padd);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int text_a = gp_text_ascent(ctx->font);
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	unsigned int i;

	const gp_text_style *font = gp_widget_focused_font(ctx, self->focused);
	gp_pixel text_color = gp_widget_text_color(self, ctx, flags);
	gp_pixel fr_color = gp_widget_frame_color(self, ctx, flags);

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_fill_rect_xywh(ctx->buf, x, y, w, h, ctx->bg_color);

	y += ctx->padd;

	size_t sel = call_get_sel(self);
	size_t cnt = call_get_cnt(self);

	for (i = 0; i < cnt; i++) {
		unsigned int r = text_a/2;
		unsigned int cy = y + r;
		unsigned int cx = x + r;

		gp_fill_circle(ctx->buf, cx, cy, r, ctx->fg_color);
		gp_fill_ring(ctx->buf, cx, cy, r - ctx->fr_thick + 1, r, fr_color);

		if (i == sel)
			gp_fill_circle(ctx->buf, cx, cy, GP_MAX((gp_size)1, r/2), text_color);

		gp_text(ctx->buf, font,
			x + ctx->padd + text_a, y,
		        GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
                        text_color, ctx->bg_color,
			call_get_choice(self, i));

		y += text_a + ctx->padd;
	}
}

static void select_choice(gp_widget *self, unsigned int sel)
{
	gp_widget_choice_sel_set_(self, sel);
	gp_widget_redraw(self);
	gp_widget_send_widget_event(self, 0);
}

static void key_up(gp_widget *self)
{
	size_t sel = call_get_sel(self);

	if (sel > 0)
		select_choice(self, sel - 1);
	else
		select_choice(self, call_get_cnt(self) - 1);
}

static void key_down(gp_widget *self)
{
	size_t sel = call_get_sel(self);
	size_t cnt = call_get_cnt(self);

	if (sel + 1 >= cnt)
		select_choice(self, 0);
	else
		select_choice(self, sel + 1);
}

static void radio_click(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	unsigned int min_x = 0;
	unsigned int max_x = self->w;
	unsigned int min_y = ctx->padd;
	unsigned int max_y = self->h - ctx->padd;
	unsigned int text_h = gp_text_ascent(ctx->font) + ctx->padd;

	if (ev->st->cursor_x < min_x || ev->st->cursor_x > max_x)
		return;

	if (ev->st->cursor_y < min_y || ev->st->cursor_y > max_y)
		return;

	unsigned int select = (ev->st->cursor_y - min_y) / text_h;

	select_choice(self, select);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	if (gp_widget_key_mod_pressed(ev))
		return 0;

	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		switch (ev->val) {
		case GP_BTN_TOUCH:
		case GP_BTN_LEFT:
			radio_click(self, ctx, ev);
			return 1;
		case GP_KEY_DOWN:
			key_down(self);
			return 1;
		case GP_KEY_UP:
			key_up(self);
			return 1;
		case GP_KEY_HOME:
			select_choice(self, 0);
			return 1;
		case GP_KEY_END:
			select_choice(self, call_get_cnt(self) - 1);
			return 1;
		}
	break;
	case GP_EV_REL:
		if (ev->code == GP_EV_REL_WHEEL) {
			if (ev->val < 0)
				key_down(self);
			else
				key_up(self);
		}
	break;
	}

	return 0;
}

static gp_widget *json_to_radiobutton(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	return gp_widget_choice_from_json(GP_WIDGET_RADIOBUTTON, json, val, ctx);
}

struct gp_widget_ops gp_widget_radiobutton_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.from_json = json_to_radiobutton,
	.id = "radiobutton",
};
