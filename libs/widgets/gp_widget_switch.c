//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

struct switch_payload {
	const char *on_label;
	const char *off_label;
	gp_widget_stock_type on_stock;
	gp_widget_stock_type off_stock;
	gp_coord max_label_width;
	char data[];
};

/*
 * Calculate how much space will be take by a stock image, if any. Includes padding before the stock.
 */
static gp_size stock_width(struct switch_payload *priv, const gp_widget_render_ctx *ctx)
{
	if (!priv->on_stock && !priv->off_stock)
		return 0;

	return gp_text_ascent(ctx->font) + 3*ctx->padd;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct switch_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(self);
	const gp_text_style *font = gp_widget_focused_font(ctx, 1);
	unsigned int text_a = gp_text_ascent(font);
	unsigned int on_text_w = 0;
	unsigned int off_text_w = 0;

	if (priv->on_label)
		on_text_w = gp_text_wbbox(font, priv->on_label) + ctx->padd;

	if (priv->off_label)
		off_text_w = gp_text_wbbox(font, priv->off_label) + ctx->padd;

	priv->max_label_width = GP_MAX(on_text_w, off_text_w);

	return 2 * text_a + 4 + 2*ctx->padd + stock_width(priv, ctx) + priv->max_label_width;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return gp_text_ascent(ctx->font) + GP_MAX(4, 2 * ctx->padd);
}

/*
 * Returns current label.
 */
static const char *current_label(gp_widget_class_bool *b, struct switch_payload *priv)
{
	return b->val ? priv->on_label : priv->off_label;
}

/*
 * Calculate how much space will be taken by the label, if any. Includes padding before the label.
 *
 * The label width is cached in the min_w() callback.
 */
static gp_size label_width(struct switch_payload *priv)
{
	if (!priv->max_label_width)
		return 0;

	return priv->max_label_width;
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(self);
	struct switch_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(self);
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

	gp_pixel switch_color = b->val ? ctx->hl_color : ctx->bg_color;

	if (gp_widget_is_disabled(self, flags))
		switch_color = ctx->bg_color;

	unsigned int switch_w = w - label_width(priv) - stock_width(priv, ctx);

	gp_fill_rrect_xywh(ctx->buf, x, y, switch_w, h, ctx->bg_color, switch_color, fr_color);

	unsigned int switch_x = x + (b->val ? 2 : switch_w - (switch_w-4)/2 - 2);
	unsigned int switch_h = h-4;

	gp_fill_rrect_xywh_focused(ctx->buf, switch_x, y+2, (switch_w-4)/2, switch_h,
	                           switch_color, ctx->fg_color, text_color, self->focused);

	unsigned int sw = GP_MIN(switch_w, switch_h)/3;
	unsigned int padd_h = switch_h/2 - sw;

	if (priv->on_stock == priv->off_stock) {
		if (b->val) {
			gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ON,
			                       x + switch_w - switch_w/4 - sw/2, y + 2 + padd_h,
					       sw, switch_h - 2 * padd_h,
					       switch_color, ctx);
		} else {
			gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_OFF,
			                       x + switch_w/4 - sw/2, y + 2 + padd_h,
					       sw, switch_h - 2 * padd_h,
					       switch_color, ctx);
		}
	}

	unsigned int cur_x = x + switch_w + ctx->padd;

	const char *label = current_label(b, priv);

	if (label) {
		gp_text(ctx->buf, font,
			cur_x, y+ctx->padd,
			GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
			text_color, ctx->bg_color, label);
	}

	cur_x += priv->max_label_width;

	gp_widget_stock_type stock_type = b->val ? priv->on_stock : priv->off_stock;

	if (stock_type) {
		gp_widget_stock_render(ctx->buf, stock_type, cur_x, y,
		                       text_a + 2 * ctx->padd, text_a + 2 * ctx->padd,
		                       ctx->bg_color, ctx);
		cur_x += text_a + 3*ctx->padd;
	}
}

static void set(gp_widget *self, int val)
{
	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(self);

	b->val = val;

	gp_widget_redraw(self);

	gp_widget_send_widget_event(self, 0);
}

static void toggle(gp_widget *self)
{
	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(self);

	set(self, !b->val);
}

static void click(gp_widget *self, gp_event *ev)
{
	unsigned int max_x = self->w;
	unsigned int max_y = self->h;

	if (ev->st->cursor_x > max_x)
		return;

	if (ev->st->cursor_y > max_y)
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
		break;
		case GP_BTN_TOUCH:
		case GP_BTN_LEFT:
			click(self, ev);
			return 1;
		break;
		}
	}

	return 0;
}

enum keys {
	LABEL,
	OFF_LABEL,
	OFF_STOCK,
	ON_LABEL,
	ON_STOCK,
	SET,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(LABEL, "label", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(OFF_LABEL, "off_label", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(OFF_STOCK, "off_stock", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(ON_LABEL, "on_label", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(ON_STOCK, "on_stock", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(SET, "set", GP_JSON_BOOL),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_switch(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	gp_widget *ret;
	char *on_label = NULL;
	char *off_label = NULL;
	gp_widget_stock_type on_stock = GP_WIDGET_STOCK_NONE;
	gp_widget_stock_type off_stock = GP_WIDGET_STOCK_NONE;
	gp_widget_stock_type type;
	int set = 0;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case LABEL:
			if (on_label || off_label) {
				gp_json_warn(json, "label cannot be set with off_label or on_label");
				continue;
			}
			off_label = strdup(val->val_str);
			on_label = strdup(val->val_str);
		break;
		case OFF_LABEL:
			if (off_label) {
				gp_json_warn(json, "off label already set");
				continue;
			}
			off_label = strdup(val->val_str);
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
		case ON_LABEL:
			if (on_label) {
				gp_json_warn(json, "on label already set");
				continue;
			}
			on_label = strdup(val->val_str);
		break;
		case SET:
			set = val->val_bool;
		break;
		}
	}

	ret = gp_widget_switch_new(on_label, on_stock, off_label, off_stock, set);

	free(off_label);
	free(on_label);

	return ret;
}

struct gp_widget_ops gp_widget_switch_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.from_json = json_to_switch,
	.id = "switch",
};

gp_widget *gp_widget_switch_new(const char *on_label, gp_widget_stock_type on_stock,
                                const char *off_label, gp_widget_stock_type off_stock,
				bool set)
{
	gp_widget *ret;
	size_t size = sizeof(gp_widget_class_bool) + sizeof(struct switch_payload);

	size += off_label ? strlen(off_label) + 1 : 0;
	size += on_label ? strlen(on_label) + 1 : 0;

	ret = gp_widget_new(GP_WIDGET_SWITCH, GP_WIDGET_CLASS_BOOL, size);
	if (!ret)
		return NULL;

	struct switch_payload *priv = GP_WIDGET_CLASS_BOOL_PAYLOAD(ret);

	priv->off_label = NULL;
	priv->off_stock = off_stock;
	priv->on_label = NULL;
	priv->on_stock = on_stock;

	char *data = priv->data;

	if (off_label) {
		priv->off_label = data;
		strcpy(data, off_label);
		data += strlen(off_label) + 1;
	}

	if (on_label) {
		priv->on_label = data;
		strcpy(data, on_label);
	}

	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(ret);
	b->val = set;

	return ret;
}
