//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int text_a = gp_text_ascent(ctx->font);
	unsigned int text_w = 0;

	if (self->b->label)
		text_w = gp_text_width(ctx->font, self->b->label) + ctx->padd;

	return text_a + text_w;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return gp_text_ascent(ctx->font) + 2 * ctx->padd;
}

static void cross(gp_pixmap *buf, unsigned int x, unsigned int y,
		  unsigned int w, unsigned int h, gp_pixel col)
{
	gp_line(buf, x + 3, y + 3, x + w - 4, y + h - 4, col);
	gp_line(buf, x + 3, y + h - 4, x + w - 4, y + 3, col);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int text_a = gp_text_ascent(ctx->font);
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	(void)flags;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_fill_rect_xywh(ctx->buf, x, y, w, h, ctx->bg_color);

	y += ctx->padd;

	gp_pixel color = self->focused ? ctx->sel_color : ctx->text_color;

	gp_fill_rrect_xywh(ctx->buf, x, y, text_a, text_a, ctx->bg_color, ctx->fg_color, color);

	if (self->b->val) {
		cross(ctx->buf, x, y,
		      text_a, text_a, ctx->text_color);
	}

	if (!self->b->label)
		return;

	gp_text(ctx->buf, ctx->font,
		x + text_a + ctx->padd, y,
		GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
		ctx->text_color,
		ctx->bg_color, self->b->label);
}

static void set(gp_widget *self, int val)
{
	self->b->val = val;

	gp_widget_redraw(self);

	gp_widget_send_widget_event(self, 0);
}

static void toggle(gp_widget *self)
{
	set(self, !self->b->val);
}

static void click(gp_widget *self, unsigned int padd, gp_event *ev)
{
	unsigned int max_x = self->w;
	unsigned int min_y = padd;
	unsigned int max_y = self->h - padd;

	if (ev->cursor_x > max_x)
		return;

	if (ev->cursor_y < min_y || ev->cursor_y > max_y)
		return;

	toggle(self);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code != GP_EV_KEY_DOWN)
			return 0;

		switch (ev->val) {
		case GP_KEY_ENTER:
			toggle(self);
			return 1;
		break;
		case GP_BTN_PEN:
		case GP_BTN_LEFT:
			click(self, ctx->padd, ev);
			return 1;
		break;
		}
	}

	return 0;
}

static gp_widget *json_to_checkbox(json_object *json, gp_htable **uids)
{
	const char *label = NULL;
	int set = 0;

	(void)uids;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "label"))
			label = json_object_get_string(val);
		else if (!strcmp(key, "set"))
			set = json_object_get_boolean(val);
		else
			GP_WARN("Invalid checkbox key '%s'", key);
	}

	return gp_widget_checkbox_new(label, set, NULL, NULL);
}

struct gp_widget_ops gp_widget_checkbox_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.from_json = json_to_checkbox,
	.id = "checkbox",
};

void gp_widget_checkbox_set(gp_widget *self, int val)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_CHECKBOX, );

	val = !!val;

	if (self->checkbox->val == val)
		return;

	set(self, val);
}

void gp_widget_checkbox_toggle(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_CHECKBOX, );

	toggle(self);
}

int gp_widget_checkbox_get(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_CHECKBOX, -1);

	return self->checkbox->val;
}

gp_widget *gp_widget_checkbox_new(const char *label, int val,
                                  int on_event(gp_widget_event *ev),
                                  void *priv)
{
	gp_widget *ret;
	size_t size = sizeof(struct gp_widget_bool);

	size += label ? strlen(label) + 1 : 0;

	ret = gp_widget_new(GP_WIDGET_CHECKBOX, GP_WIDGET_CLASS_BOOL, size);
	if (!ret)
		return NULL;

	if (label) {
		ret->b->label = ret->b->payload;
		strcpy(ret->b->payload, label);
	}

	ret->b->val = !!val;

	gp_widget_event_handler_set(ret, on_event, priv);

	return ret;
}
