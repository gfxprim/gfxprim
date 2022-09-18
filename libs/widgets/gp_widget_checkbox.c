//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int text_a = gp_text_ascent(ctx->font);
	unsigned int text_w = 0;

	if (self->b->label)
		text_w = gp_text_wbbox(ctx->font, self->b->label) + ctx->padd;

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
	gp_pixel text_color = ctx->text_color;

	if (gp_widget_is_disabled(self, flags))
		text_color = ctx->col_disabled;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_fill_rect_xywh(ctx->buf, x, y, w, h, ctx->bg_color);

	y += ctx->padd;

	gp_pixel color = self->focused ? ctx->sel_color : text_color;

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
		text_color, ctx->bg_color, self->b->label);
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

	if (ev->st->cursor_x > max_x)
		return;

	if (ev->st->cursor_y < min_y || ev->st->cursor_y > max_y)
		return;

	toggle(self);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	if (gp_widget_key_mod_pressed(ev))
		return 0;

	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code != GP_EV_KEY_DOWN)
			return 0;

		switch (ev->val) {
		case GP_KEY_ENTER:
		case GP_KEY_SPACE:
			toggle(self);
			return 1;
		break;
		case GP_BTN_TOUCH:
		case GP_BTN_LEFT:
			click(self, ctx->padd, ev);
			return 1;
		break;
		}
	}

	return 0;
}

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("label", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("set", GP_JSON_BOOL),
};

enum keys {
	LABEL,
	SET,
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_checkbox(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	gp_widget *ret;
	char *label = NULL;
	int set = 0;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case LABEL:
			label = strdup(val->val_str);
		break;
		case SET:
			set = val->val_bool;
		break;
		}
	}

	ret = gp_widget_checkbox_new(label, set, NULL, NULL);

	free(label);

	return ret;
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

	gp_widget_on_event_set(ret, on_event, priv);

	return ret;
}
