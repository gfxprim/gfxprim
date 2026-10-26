//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2026 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_json.h>

struct push_button_payload {
	char *label;
	gp_widget_stock_type on_stock;
	gp_widget_stock_type off_stock;
};

/*
 * Space for a stock image is reserved if there is one for either state so
 * that the label does not move when the button is toggled.
 */
static int has_stock(struct push_button_payload *priv)
{
	return priv->on_stock || priv->off_stock;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct push_button_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(self);
	const gp_text_style *font = gp_widget_focused_font(ctx, 1);
	unsigned int ret = 2 * ctx->padd;

	if (priv->label)
		ret += gp_text_wbbox(font, priv->label);

	if (has_stock(priv))
		ret += GP_ODD_UP(gp_text_ascent(font));

	if (priv->label && has_stock(priv))
		ret += gp_text_wbbox(font, " ");

	return ret;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return 2 * ctx->padd + GP_ODD_UP(gp_text_ascent(ctx->font));
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(self);
	struct push_button_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(self);
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	const gp_text_style *font = gp_widget_focused_font(ctx, self->focused);
	gp_pixel text_color = gp_widget_text_color(self, ctx, flags);
	gp_pixel fr_color = gp_widget_frame_color(self, ctx, flags);
	gp_pixel bg_color = b->val ? ctx->hl_color : ctx->fg_color;

	if (gp_widget_is_disabled(self, flags))
		bg_color = b->val ? ctx->fg_color : ctx->bg_color;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, bg_color, fr_color);

	unsigned int asc = gp_text_ascent(ctx->font);
	unsigned int asc_half = asc/2;
	unsigned int spc = gp_text_wbbox(ctx->font, " ");
	unsigned int cx = x + w/2;
	unsigned int cy = y + h/2;

	if (priv->label) {
		unsigned int tcx = cx;
		unsigned int len;

		if (has_stock(priv))
			tcx += asc/2 + spc/2;

		len = gp_text(ctx->buf, font,
		              tcx, cy-(asc-asc_half), GP_ALIGN_CENTER|GP_VALIGN_BELOW,
		              text_color, bg_color, priv->label);

		cx -= len/2 + spc/2;
	}

	gp_widget_stock_type type = b->val ? priv->on_stock : priv->off_stock;

	if (!type)
		return;

	gp_size sw = GP_ODD_UP(asc);

	if (gp_widget_is_disabled(self, flags))
		type |= GP_WIDGET_STOCK_DISABLED;

	gp_widget_stock_render(ctx->buf, type, cx - sw/2, cy - (sw+1)/2,
	                       sw, sw, bg_color, ctx);
}

static void toggle(gp_widget *self)
{
	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(self);

	b->val = !b->val;

	gp_widget_redraw(self);

	gp_widget_send_widget_event(self, 0);
}

static void click(gp_widget *self, gp_event *ev)
{
	if (ev->st->cursor_x > self->w)
		return;

	if (ev->st->cursor_y > self->h)
		return;

	toggle(self);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	(void) ctx;

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
		case GP_BTN_TOUCH:
		case GP_BTN_LEFT:
			click(self, ev);
			return 1;
		}
	}

	return 0;
}

static void free_(gp_widget *self)
{
	struct push_button_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(self);

	free(priv->label);
}

enum keys {
	LABEL,
	OFF_STOCK,
	ON_STOCK,
	SET,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(LABEL, "label", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(OFF_STOCK, "off_stock", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(ON_STOCK, "on_stock", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(SET, "set", GP_JSON_BOOL),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_push_button(gp_json_reader *json, gp_json_val *val,
                                      gp_widget_json_ctx *ctx)
{
	gp_widget *ret;
	char *label = NULL;
	gp_widget_stock_type on_stock = GP_WIDGET_STOCK_NONE;
	gp_widget_stock_type off_stock = GP_WIDGET_STOCK_NONE;
	gp_widget_stock_type type;
	int set = 0;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case LABEL:
			if (label)
				gp_json_warn(json, "Duplicated label!");
			else
				label = strdup(val->val_str);
		break;
		case OFF_STOCK:
		case ON_STOCK:
			type = gp_widget_stock_type_by_name(val->val_str);
			if (type == GP_WIDGET_STOCK_TYPE_INVALID) {
				gp_json_warn(json, "Unknown stock type!");
				continue;
			}

			if (val->idx == OFF_STOCK)
				off_stock = type;
			else
				on_stock = type;
		break;
		case SET:
			set = val->val_bool;
		break;
		}
	}

	ret = gp_widget_push_button_new(label, on_stock, off_stock, set);

	free(label);

	return ret;
}

struct gp_widget_ops gp_widget_push_button_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.free = free_,
	.from_json = json_to_push_button,
	.id = "push_button",
};

gp_widget *gp_widget_push_button_new(const char *label,
                                     gp_widget_stock_type on_stock,
                                     gp_widget_stock_type off_stock,
                                     bool set)
{
	gp_widget *ret;
	size_t size = sizeof(gp_widget_class_bool) + sizeof(struct push_button_payload);

	ret = gp_widget_new(GP_WIDGET_PUSH_BUTTON, GP_WIDGET_CLASS_BOOL, size);
	if (!ret)
		return NULL;

	struct push_button_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(ret);

	priv->label = NULL;
	priv->on_stock = on_stock;
	priv->off_stock = off_stock;

	if (label) {
		priv->label = strdup(label);
		if (!priv->label) {
			free(ret);
			return NULL;
		}
	}

	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(ret);
	b->val = set;

	return ret;
}

void gp_widget_push_button_label_set(gp_widget *self, const char *label)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_PUSH_BUTTON, );
	struct push_button_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(self);
	char *new_label = NULL;

	if (label) {
		new_label = strdup(label);
		if (!new_label)
			return;
	}

	free(priv->label);
	priv->label = new_label;

	gp_widget_redraw(self);
	gp_widget_resize(self);
}

const char *gp_widget_push_button_label_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_PUSH_BUTTON, NULL);
	struct push_button_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(self);

	return priv->label;
}
