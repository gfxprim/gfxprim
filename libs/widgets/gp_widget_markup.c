//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <utils/gp_vec_str.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_markup_parser.h>

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
			width += gp_text_wbbox(font, str);

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
			width += gp_text_wbbox(font, str);

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
	gp_pixel text_color = ctx->text_color;

	if (gp_widget_is_disabled(self, flags))
		text_color = ctx->col_disabled;

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

			gp_pixel fg = text_color;
			gp_pixel bg = ctx->bg_color;

			if (e->attrs & GP_MARKUP_SUBSCRIPT)
				cur_y += ctx->padd - gp_text_descent(font);

			if (e->attrs & GP_MARKUP_SUPERSCRIPT)
				cur_y -= gp_text_descent(font);

			if (e->attrs & GP_MARKUP_INVERSE) {
				unsigned int str_h = gp_text_ascent(font) + ctx->padd;
				unsigned int str_w = gp_text_wbbox(font, str);
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

enum keys {
	GET,
	TEXT,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("get", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("text", GP_JSON_STR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_markup(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	char *(*get)(unsigned int var_id, char *old_val) = NULL;
	gp_widget *ret = NULL;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case GET:
			get = gp_widget_callback_addr(val->val_str, ctx);
		break;
		case TEXT:
			ret = gp_widget_markup_new(val->val_str, NULL);
		break;
		}
	}

	if (!ret) {
		GP_WARN("Missing markup");
		return NULL;
	}

	ret->markup->get = get;

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
