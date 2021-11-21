//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_string.h>

static unsigned int buttons_width(const gp_widget_render_ctx *ctx)
{
	return GP_EVEN_UP(((3 * ctx->padd + gp_text_ascent(ctx->font))/2));
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int i, max_len = 0;

	for (i = 0; i < self->choice->max; i++) {
		unsigned int len = gp_text_wbbox(ctx->font, self->choice->choices[i]);
		max_len = GP_MAX(max_len, len);
	}

	return 2 * ctx->padd + max_len + buttons_width(ctx);
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return 2 * ctx->padd + gp_text_ascent(ctx->font);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	unsigned int s = buttons_width(ctx);
	unsigned int sx = (gp_text_ascent(ctx->font)/2 + ctx->padd)/3;
	unsigned int sy = (gp_text_ascent(ctx->font)/2 + ctx->padd)/5;

	(void)flags;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_pixel color = self->focused ? ctx->sel_color : ctx->text_color;

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h,
	                   ctx->bg_color, ctx->fg_color, color);

	gp_print(ctx->buf, ctx->font, x + ctx->padd, y + ctx->padd,
		 GP_ALIGN_RIGHT | GP_VALIGN_BELOW,
		 ctx->text_color, ctx->bg_color, "%s", self->choice->choices[self->choice->sel]);

	gp_coord rx = x + w - s;

	gp_vline_xyh(ctx->buf, rx-1, y, h, color);
	gp_hline_xyw(ctx->buf, rx, y + h/2, s, color);

	if (self->choice->sel == 0)
		color = ctx->bg_color;
	else
		color = ctx->text_color;

	gp_symbol(ctx->buf, x + w - s/2 - 1, y + h/4, sx, sy, GP_TRIANGLE_UP, color);

	if (self->choice->sel + 1 >= self->choice->max)
		color = ctx->bg_color;
	else
		color = ctx->text_color;

	gp_symbol(ctx->buf, x + w - s/2 - 1, y + (3*h)/4, sx, sy, GP_TRIANGLE_DOWN, color);
}

static void select_choice(gp_widget *self, unsigned int select)
{
	if (self->choice->sel == select)
		return;

	self->choice->sel = select;

	gp_widget_redraw(self);

	gp_widget_send_widget_event(self, 0);
}

static int key_up(gp_widget *self)
{
	if (self->choice->sel == 0)
		return 0;

	select_choice(self, self->choice->sel - 1);
	return 1;
}

static int key_down(gp_widget *self)
{
	if (self->choice->sel + 1 >= self->choice->max)
		return 0;

	select_choice(self, self->choice->sel + 1);
	return 1;
}

static void click(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	unsigned int s = buttons_width(ctx);
	unsigned int min_x = self->w - s;
	unsigned int max_x = self->w;
	unsigned int max_y = self->h;
	unsigned int mid_y = max_y / 2;

	if (ev->st->cursor_x < min_x || ev->st->cursor_x > max_x)
		return;

	if (ev->st->cursor_y > max_y)
		return;

	if (ev->st->cursor_y < mid_y)
		key_up(self);
	else
		key_down(self);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		switch (ev->val) {
		case GP_BTN_PEN:
		case GP_BTN_LEFT:
			click(self, ctx, ev);
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
			select_choice(self, self->choice->max - 1);
			return 1;
		}
	break;
	case GP_EV_REL:
		if (ev->code != GP_EV_REL_WHEEL)
			return 0;

		if (ev->val < 0)
			return key_down(self);

		if (ev->val > 0)
			return key_up(self);
	break;
	}

	return 0;
}

static gp_widget *json_to_spinbutton(gp_json_buf *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	return gp_widget_choice_from_json(GP_WIDGET_SPINBUTTON, json, val, ctx);
}

struct gp_widget_ops gp_widget_spinbutton_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.from_json = json_to_spinbutton,
	.id = "spinbutton",
};
