//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <utils/gp_vec_str.h>

#include <gp_widgets.h>
#include <gp_widget_ops.h>
#include <gp_widget_render.h>
#include <gp_markup_parser.h>

static gp_text_style *get_font(const gp_widget_render_ctx *ctx, int attrs)
{
	if (attrs & GP_MARKUP_BIG) {
		if (attrs & GP_MARKUP_BOLD)
			return ctx->font_big_bold;
		else
			return ctx->font_big;
	}

	if (attrs & GP_MARKUP_BOLD)
		return ctx->font_bold;

	return ctx->font;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int max_width = 0;
	unsigned int width = 0;
	gp_markup_elem *e;

	for (e = gp_markup_first(self->markup->markup); e; e = gp_markup_next(e)) {
		const gp_text_style *font = get_font(ctx, e->attrs);
		const char *str = gp_markup_elem_str(e);

		if (str)
			width += gp_text_width(font, str);

		if (e->type == GP_MARKUP_NEWLINE) {
			max_width = GP_MAX(max_width, width);
			width = 0;
		}
	}

	return GP_MAX(max_width, width);
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int height = 0;
	unsigned int max_h = 0;
	gp_markup_elem *e;

	for (e = gp_markup_first(self->markup->markup); e; e = gp_markup_next(e)) {
		const gp_text_style *font = get_font(ctx, e->attrs);

		switch (e->type) {
		case GP_MARKUP_STR:
		case GP_MARKUP_VAR:
			max_h = GP_MAX(max_h, gp_text_ascent(font));
		break;
		case GP_MARKUP_NEWLINE:
			//max_h = GP_MAX(max_h, gp_text_ascent(font));
			height += max_h + ctx->padd;
			max_h = 0;
		break;
		}
	}

	return ctx->padd + height + max_h + (max_h ? ctx->padd : 0);
}

static void line_bbox(const gp_markup_elem *e, const gp_widget_render_ctx *ctx,
                      gp_size *w, gp_size *h)
{
	gp_size width = 0;
	gp_size height = 0;

	while (e->type != GP_MARKUP_END && e->type != GP_MARKUP_NEWLINE) {
		const gp_text_style *font = get_font(ctx, e->attrs);
		const char *str = gp_markup_elem_str(e);

		if (str)
			width += gp_text_width(font, str);

		height = GP_MAX(height, gp_text_ascent(font));

		e++;
	}

	*w = width;
	*h = height;
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	(void) flags;

	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	gp_widget_ops_blit(ctx, x, y, w, h);
	gp_fill_rect_xywh(ctx->buf, x, y, w, h, ctx->bg_color);

	gp_markup_elem *e = gp_markup_first(self->markup->markup);

	for (;;) {
		gp_coord cur_x = x;
		gp_size w, h;

		line_bbox(e, ctx, &w, &h);

		while (e->type != GP_MARKUP_NEWLINE) {
			const gp_text_style *font = get_font(ctx, e->attrs);
			const char *str = gp_markup_elem_str(e);
			unsigned int cur_y = y + h + ctx->padd;

			if (e->type == GP_MARKUP_END)
				return;

			gp_pixel fg = ctx->text_color;
			gp_pixel bg = ctx->bg_color;

			if (e->attrs & GP_MARKUP_SUBSCRIPT)
				cur_y += ctx->padd - gp_text_descent(font);

			if (e->attrs & GP_MARKUP_SUPERSCRIPT)
				cur_y -= gp_text_descent(font);

			if (e->attrs & GP_MARKUP_INVERSE) {
				unsigned int str_h = gp_text_ascent(font) + ctx->padd;
				unsigned int str_w = gp_text_width(font, str);
				unsigned int str_y = cur_y + ctx->padd - str_h;

				GP_SWAP(fg, bg);

				gp_fill_rect_xywh(ctx->buf, cur_x, str_y,
				                  str_w, gp_text_height(font), bg);
			}

			cur_x += gp_text(ctx->buf, font, cur_x, cur_y,
			                 GP_ALIGN_RIGHT | GP_VALIGN_BASELINE,
			                 fg, bg, str);

			e++;
		}

		y += ctx->padd + h;
		e++;
	}
}

void gp_widget_markup_refresh(gp_widget *self)
{
	unsigned int var_id = 0;
	gp_markup_elem *e;

	if (!self->markup->get)
		return;

	for (e = gp_markup_first(self->markup->markup); e; e = gp_markup_next(e)) {
		if (e->type != GP_MARKUP_VAR)
			continue;

		e->var = self->markup->get(var_id++, e->var);
	}

	gp_widget_resize(self);
	gp_widget_redraw(self);
}

static gp_widget *json_to_markup(json_object *json, gp_htable **uids)
{
	const char *markup = NULL;
	char *(*get)(unsigned int var_id, char *old_val) = NULL;

	(void)uids;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "text"))
			markup = json_object_get_string(val);
		else if (!strcmp(key, "get"))
			get = gp_widget_callback_addr(json_object_get_string(val));
		else
			GP_WARN("Invalid markup key '%s'", key);
	}

	if (!markup) {
		GP_WARN("Missing markup");
		markup = "Missing markup";
	}

	gp_widget *ret = gp_widget_markup_new(markup, get);

	return ret;
}

struct gp_widget_ops gp_widget_markup_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.from_json = json_to_markup,
	.id = "markup",
};

gp_widget *gp_widget_markup_new(const char *markup_str,
                                char *(*get)(unsigned int var_id, char *old_val))
{
	size_t payload_size = sizeof(struct gp_widget_markup);
	gp_widget *ret;
	gp_markup *markup = gp_markup_parse(markup_str);

	if (!markup)
		return NULL;

	ret = gp_widget_new(GP_WIDGET_MARKUP, GP_WIDGET_CLASS_NONE, payload_size);
	if (!ret) {
		gp_markup_free(markup);
		return NULL;
	}

	ret->markup->get = get;
	ret->markup->markup = markup;

	ret->no_shrink = 1;

	return ret;
}

static gp_markup_elem *get_var_by_id(gp_widget *self, unsigned int var_id)
{
	unsigned int cur_id = 0;
	gp_markup_elem *e;

	for (e = gp_markup_first(self->markup->markup); e; e = gp_markup_next(e)) {
		if (e->type != GP_MARKUP_VAR)
			continue;

		if (cur_id == var_id)
			return e;

		cur_id++;
	}

	return NULL;
}

void gp_widget_markup_set_var(gp_widget *self, unsigned int var_id, const char *fmt, ...)
{
	gp_markup_elem *var;
	va_list va;

	GP_WIDGET_ASSERT(self, GP_WIDGET_MARKUP, );

	var = get_var_by_id(self, var_id);
	if (!var) {
		GP_WARN("Markup %p invalid variable id %u", self, var_id);
		return;
	}

	va_start(va, fmt);
	var->var = gp_vec_vprintf(var->var, fmt, va);
	va_end(va);

	gp_widget_resize(self);
	gp_widget_redraw(self);
}
