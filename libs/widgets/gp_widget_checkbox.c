//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

struct checkbox_payload {
	char *label;
	char data[];
};

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct checkbox_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(self);
	const gp_text_style *font = gp_widget_focused_font(ctx, 1);
	unsigned int text_a = gp_text_ascent(font);
	unsigned int text_w = 0;

	if (priv->label)
		text_w = gp_text_wbbox(font, priv->label) + ctx->padd;

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
	if (GP_MIN(w, h) < 10) {
		gp_line(buf, x + 3, y + 3, x + w - 4, y + h - 4, col);
		gp_line(buf, x + 3, y + h - 4, x + w - 4, y + 3, col);
		return;
	}

	gp_size sp = GP_MAX((gp_size)1, GP_MIN(w/4, h/4));
	gp_size th = GP_MAX((gp_size)1, GP_MIN(w/8, h/8));

	x+=sp;
	y+=sp;
	w-=2*sp+1;
	h-=2*sp+1;

	gp_size bw = (w-2*th)/2;
	gp_size bh = (h-2*th)/2;

	gp_coord poly[] = {
		0, 0,
		th, 0,
		w/2, bh,
		w/2 + w%2, bh,
		w - th, 0,
		w, 0,
		w, th,
		w - bw, h/2,
		w - bw, h/2 + h%2,
		w, h - th,
		w, h,
		w - th, h,
		w/2 + w%2, h - bh,
		w/2, h - bh,
		th, h,
		0, h,
		0, h - th,
		0 + bw, h/2 + h%2,
		0 + bw, h/2,
		0, th,
	};

	gp_fill_polygon(buf, x, y, GP_ARRAY_SIZE(poly)/2, poly, col);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct checkbox_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(self);
	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(self);
	unsigned int text_a = gp_text_ascent(ctx->font);
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	const gp_text_style *font = gp_widget_focused_font(ctx, self->focused);
	gp_pixel text_color = gp_widget_text_color(self, ctx, flags);
	gp_pixel fr_color = gp_widget_frame_color(self, ctx, flags);

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_fill_rect_xywh(ctx->buf, x, y, w, h, ctx->bg_color);

	y += ctx->padd;

	gp_fill_rrect_xywh(ctx->buf, x, y, text_a, text_a, ctx->bg_color, ctx->fg_color, fr_color);

	if (b->val) {
		cross(ctx->buf, x, y,
		      text_a, text_a, text_color);
	}

	if (!priv->label)
		return;

	gp_text(ctx->buf, font,
		x + text_a + ctx->padd, y,
		GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
		text_color, ctx->bg_color, priv->label);
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

	gp_widget_bool_toggle(self);
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
			gp_widget_bool_toggle(self);
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

enum keys {
	LABEL,
	SET,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(LABEL, "label", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(SET, "set", GP_JSON_BOOL),
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

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case LABEL:
			label = strdup(val->val_str);
		break;
		case SET:
			set = val->val_bool;
		break;
		}
	}

	ret = gp_widget_checkbox_new(label, set);

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

gp_widget *gp_widget_checkbox_new(const char *label, bool val)
{
	gp_widget *ret;
	size_t size = sizeof(gp_widget_class_bool) + sizeof(struct checkbox_payload);

	size += label ? strlen(label) + 1 : 0;

	ret = gp_widget_new(GP_WIDGET_CHECKBOX, GP_WIDGET_CLASS_BOOL, size);
	if (!ret)
		return NULL;

	struct checkbox_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(ret);

	if (label) {
		priv->label = priv->data;
		strcpy(priv->label, label);
	}

	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(ret);

	b->val = val;

	return ret;
}

const char *gp_widget_checkbox_label_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_CHECKBOX, NULL);
	struct checkbox_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(self);

	return priv->label;
}
