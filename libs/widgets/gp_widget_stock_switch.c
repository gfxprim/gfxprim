//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

struct switch_priv {
	gp_widget_size min_size;
	gp_widget_stock_type on_stock;
	gp_widget_stock_type off_stock;
};

#define SWITCH_PRIV(widget) ((struct switch_priv *)(GP_WIDGET_CLASS_BOOL(widget)->payload))

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct switch_priv *priv = SWITCH_PRIV(self);

	return GP_ODD_UP(gp_widget_size_units_get(&priv->min_size, ctx));
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct switch_priv *priv = SWITCH_PRIV(self);

	return GP_ODD_UP(gp_widget_size_units_get(&priv->min_size, ctx));
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(self);
	struct switch_priv *priv = SWITCH_PRIV(self);
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	gp_pixel fr_color = gp_widget_frame_color(self, ctx, flags);
	gp_pixel bg_color = ctx->fg_color;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, bg_color, fr_color);

	gp_widget_stock_type stock_type = b->val ? priv->on_stock : priv->off_stock;

	x+=ctx->padd/2;
	y+=ctx->padd/2;

	w-=2*(ctx->padd/2);
	h-=2*(ctx->padd/2);

	gp_size stock_s = GP_MIN(w, h);

	x += (w-stock_s)/2;
	y += (h-stock_s)/2;

	gp_widget_stock_render(ctx->buf, stock_type, x, y, stock_s, stock_s, bg_color, ctx);
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
	set(self, !self->b->val);
}

static void click(gp_widget *self, unsigned int padd, gp_event *ev)
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
	GP_JSON_OBJ_ATTR("min_size", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("off_stock", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("on_stock", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("set", GP_JSON_BOOL),
};

enum keys {
	MIN_SIZE,
	OFF_STOCK,
	ON_STOCK,
	SET,
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_stock_switch(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	gp_widget *ret;
	gp_widget_size min_size = GP_WIDGET_SIZE_DEFAULT;
	gp_widget_stock_type on_stock = GP_WIDGET_STOCK_NONE;
	gp_widget_stock_type off_stock = GP_WIDGET_STOCK_NONE;
	gp_widget_stock_type type;
	int set = 0;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case MIN_SIZE:
			if (gp_widget_size_units_parse(val->val_str, &min_size))
				gp_json_warn(json, "Invalid size string!");
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

	ret = gp_widget_stock_switch_new(on_stock, off_stock, min_size, set);

	return ret;
}

struct gp_widget_ops gp_widget_stock_switch_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.from_json = json_to_stock_switch,
	.id = "stock_switch",
};

gp_widget *gp_widget_stock_switch_new(gp_widget_stock_type on_stock,
                                      gp_widget_stock_type off_stock,
				      gp_widget_size min_size,
				      bool set)
{
	gp_widget *ret;
	size_t size = sizeof(gp_widget_class_bool) + sizeof(struct switch_priv);

	ret = gp_widget_new(GP_WIDGET_STOCK_SWITCH, GP_WIDGET_CLASS_BOOL, size);
	if (!ret)
		return NULL;

	struct switch_priv *priv = SWITCH_PRIV(ret);

	priv->off_stock = off_stock;
	priv->on_stock = on_stock;

	if (GP_WIDGET_SIZE_EQ(min_size, GP_WIDGET_SIZE_DEFAULT))
		priv->min_size = GP_WIDGET_SIZE(0, 2, 1);
	else
		priv->min_size = min_size;

	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(ret);
	b->val = set;

	return ret;
}
