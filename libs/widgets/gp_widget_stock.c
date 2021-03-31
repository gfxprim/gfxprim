//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <gp_widgets.h>

static void redraw_stock_err_warn(gp_widget_event *ev, gp_pixel col)
{
	gp_pixmap *pix = ev->self->pixmap->pixmap;
	const gp_widget_render_ctx *ctx = ev->ctx;

	gp_coord cx = pix->w / 2;
	gp_coord cy = pix->h / 2;
	gp_size c = pix->w/2 - 2;
	gp_size r = c/5;

	gp_fill_rrect_xywh(pix, 0, 0, pix->w, pix->h, ctx->bg_color, col, ctx->text_color);

	if (r > 1)
		gp_fill_circle(pix, cx, cy+cy/2, r, ctx->text_color);
	else
		gp_fill_rect_xyxy(pix, cx-r, cy+cy/2-r, cx+r, cy+cy/2+r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-r, cy-cy/2-r, cx+r, cy+r, ctx->text_color);
}

static int redraw_stock_err(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_REDRAW)
		return 0;

	redraw_stock_err_warn(ev, ev->ctx->alert_color);

	return 1;
}

static int redraw_stock_warn(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_REDRAW)
		return 0;

	redraw_stock_err_warn(ev, ev->ctx->warn_color);

	return 1;
}

static int redraw_stock_info(gp_widget_event *ev)
{
	gp_pixmap *pix = ev->self->pixmap->pixmap;
	const gp_widget_render_ctx *ctx = ev->ctx;

	if (ev->type != GP_WIDGET_EVENT_REDRAW)
		return 0;

	gp_coord cx = pix->w / 2;
	gp_coord cy = pix->h / 2;
	gp_size c = pix->w/2 - 2;
	gp_size r = c/5;

	gp_fill_rrect_xywh(pix, 0, 0, pix->w, pix->h, ctx->bg_color, ctx->sel_color, ctx->text_color);

	if (r > 1)
		gp_fill_circle(pix, cx, cy-cy/2, r, ctx->text_color);
	else
		gp_fill_rect_xyxy(pix, cx-r, cy-cy/2-r, cx+r, cy-cy/2+r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-r, cy+cy/2, cx+r, cy-r, ctx->text_color);

	return 1;
}

static int redraw_stock_question(gp_widget_event *ev)
{
	gp_pixmap *pix = ev->self->pixmap->pixmap;
	const gp_widget_render_ctx *ctx = ev->ctx;

	if (ev->type != GP_WIDGET_EVENT_REDRAW)
		return 0;

	gp_coord cx = pix->w / 2;
	gp_coord cy = pix->h / 2;
	gp_size c = pix->w/2 - 2;
	gp_size r = c/5;

	gp_fill_rrect_xywh(pix, 0, 0, pix->w, pix->h, ctx->bg_color, ctx->sel_color, ctx->text_color);

	if (r > 1)
		gp_fill_circle(pix, cx, cy+cy/2+r, r, ctx->text_color);
	else
		gp_fill_rect_xyxy(pix, cx-r, cy+cy/2, cx+r, cy+cy/2+2*r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-r, cy-cy/2+3*r, cx+r, cy+2*r, ctx->text_color);
	gp_fill_ring_seg(pix, cx, cy-cy/2 + r, r, 3*r, GP_CIRCLE_SEG1 | GP_CIRCLE_SEG2 |GP_CIRCLE_SEG4, ctx->text_color);
	gp_fill_triangle(pix, cx-r, cy-cy/2+3*r, cx, cy-r+1, cx, cy, ctx->text_color);

	return 1;
}

gp_widget *gp_widget_stock_new(enum gp_widget_stock_types type)
{
	const gp_widget_render_ctx *ctx = gp_widgets_render_ctx();
	gp_widget *ret;
	gp_size w = GP_ODD_UP(gp_text_ascent(ctx->font) + 2 * ctx->padd);

	switch (type) {
	case GP_WIDGET_STOCK_ERR:
		ret = gp_widget_pixmap_new(w, w, redraw_stock_err, NULL);
	break;
	case GP_WIDGET_STOCK_WARN:
		ret = gp_widget_pixmap_new(w, w, redraw_stock_warn, NULL);
	break;
	case GP_WIDGET_STOCK_INFO:
		ret = gp_widget_pixmap_new(w, w, redraw_stock_info, NULL);
	break;
	case GP_WIDGET_STOCK_QUESTION:
		ret = gp_widget_pixmap_new(w, w, redraw_stock_question, NULL);
	break;
	default:
		GP_WARN("Invalid stock type %i\n", type);
		return NULL;
	}

	if (!ret)
		return NULL;

	gp_widget_event_unmask(ret, GP_WIDGET_EVENT_REDRAW);

	return ret;
}

static struct stock_types {
	const char *str_type;
	int type;
} stock_types[] = {
	{"err", GP_WIDGET_STOCK_ERR},
	{"warn", GP_WIDGET_STOCK_WARN},
	{"info", GP_WIDGET_STOCK_INFO},
	{"question", GP_WIDGET_STOCK_QUESTION}
};

static int gp_widget_stock_type_from_str(const char *type)
{
	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(stock_types); i++) {
		if (!strcmp(type, stock_types[i].str_type))
			return stock_types[i].type;
	}

	return -1;
}

gp_widget *gp_widget_stock_from_json(json_object *json, void **uids)
{
	const char *stock = NULL;
	int type;

	(void)uids;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "stock"))
			stock = json_object_get_string(val);
		else
			GP_WARN("Invalid stock key '%s'", key);
	}

	if (!stock) {
		GP_WARN("Missing stock type");
		return NULL;
	}

	type = gp_widget_stock_type_from_str(stock);
	if (type < 0) {
		GP_WARN("Unknown stock '%s'", stock);
		return NULL;
	}

	return gp_widget_stock_new(type);
}
