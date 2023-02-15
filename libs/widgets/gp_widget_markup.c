//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <utils/gp_vec_str.h>

#include <utils/gp_markup.h>
#include <utils/gp_markup_justify.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

static const gp_text_style *get_font(const gp_widget_render_ctx *ctx, int attrs)
{
	if (attrs & GP_MARKUP_LARGE) {
		if (attrs & GP_MARKUP_BOLD)
			return ctx->font_big_bold;
		else
			return ctx->font_big;
	}

	if (attrs & GP_MARKUP_MONO) {
		if (attrs & GP_MARKUP_BOLD)
			return ctx->font_mono_bold;
		else
			return ctx->font_mono;
	}

	if (attrs & GP_MARKUP_BOLD)
		return ctx->font_bold;

	return ctx->font;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int res = GP_MAX(self->markup->min_size_em, 20);

	return gp_text_avg_width(ctx->font, res);
}

#define FONT_MASK (GP_MARKUP_LARGE | GP_MARKUP_BOLD | GP_MARKUP_MONO)

static unsigned int markup_width_cb(gp_markup_glyph *first, size_t len, void *priv)
{
	size_t i, start = 0;
	unsigned int ret = 0;
	int fmt = first->fmt;

	for (i = 0; i < len; i++) {
		if ((fmt & FONT_MASK) != (first[i].fmt & FONT_MASK)) {
			const gp_text_style *font = get_font(priv, fmt);

			while (start < i) {
				ret += gp_glyph_advance_x(font, first[start].glyph);
				start++;
			}

			fmt = first[i].fmt;
		}
	}

	const gp_text_style *font = get_font(priv, fmt);

	while (start < i) {
		ret += gp_glyph_advance_x(font, first[start].glyph);
		start++;
	}

	return ret;
}

static unsigned int line_height(const gp_markup_line *line, const gp_widget_render_ctx *ctx)
{
	const gp_markup_glyph *i;
	const gp_text_style *prev = NULL, *cur;
	unsigned int height = 0;

	if (!line->first) {
		if (line->last->fmt & GP_MARKUP_STRIKE)
			return 2 * ctx->padd;

		return ctx->padd;
	}

	for (i = line->first; i <= line->last; i++) {
		cur = get_font(ctx, i->fmt);

		if (cur != prev) {
			height = GP_MAX(height, gp_text_ascent(cur));
			cur = prev;
		}
	}

	return height + ctx->padd;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int i, height = 0;

	free(self->markup->lines);

	self->markup->lines = gp_markup_justify(self->markup->markup, self->w, markup_width_cb, ctx);
	if (!self->markup->lines)
		return height;

	for (i = 0; i < self->markup->lines->lines_cnt; i++)
		height += line_height(&(self->markup->lines->lines[i]), ctx);

	return height;
}

static inline gp_pixel glyph_color(const gp_widget_render_ctx *ctx, const gp_markup_glyph *g)
{
	if (!g->fg_color || g->glyph == ' ')
		return ctx->text_color;

	return gp_widgets_color(ctx, GP_WIDGETS_THEME_COLORS + g->fg_color);
}

//TODO: cache
static gp_size max_line_ascend(const gp_widget_render_ctx *ctx, const gp_markup_line *line)
{
	gp_size ret = 0;
	const gp_markup_glyph *i;

	for (i = line->first; i && i <= line->last; i++) {
		const gp_text_style *font = get_font(ctx, i->fmt);
		ret = GP_MAX(ret, gp_text_ascent(font));
	}

	return ret;
}

static gp_size render_line(const gp_markup_line *line, gp_widget *self,
                           const gp_widget_render_ctx *ctx,
                           gp_size cur_x, gp_size cur_y)
{
	const gp_markup_glyph *i;
	const gp_text_style *font;
	int flags = GP_VALIGN_BASELINE;
	gp_size max_ascend = max_line_ascend(ctx, line);

	for (i = line->first; i && i <= line->last; i++) {
		font = get_font(ctx, i->fmt);
		gp_size y = cur_y + max_ascend;
		gp_pixel fg = glyph_color(ctx, i);

		if (i->fmt & GP_MARKUP_SUB)
			y += gp_text_descent(font);

		if (i->fmt & GP_MARKUP_SUP)
			y -= gp_text_descent(font);

		gp_size x = cur_x;

		gp_size glyph_advance = gp_glyph_draw(ctx->buf, font, x, y, flags,
				                      fg, ctx->bg_color, i->glyph);

		if (i->fmt & GP_MARKUP_UNDERLINE) {
			unsigned int uy = y + gp_text_descent(font)/3 + 1;
			unsigned int line_h = gp_text_height(font)/25 + 1;

			gp_fill_rect_xywh(ctx->buf, cur_x, uy, gp_glyph_advance_x(font, i->glyph), line_h, fg);
		}

		if (i->fmt & GP_MARKUP_STRIKE) {
			unsigned int sy = y - gp_text_ascent(font)/3;
			unsigned int line_h = gp_text_height(font)/25 + 1;

			gp_fill_rect_xywh(ctx->buf, cur_x, sy, gp_glyph_advance_x(font, i->glyph), line_h, fg);
		}

		cur_x += glyph_advance;

		flags |= GP_TEXT_BEARING;
	}

	return max_ascend + ctx->padd;
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

	unsigned int i;
	gp_coord cur_y = y;
	const gp_markup_lines *lines = self->markup->lines;

	for (i = 0; i < lines->lines_cnt; i++) {
		cur_y += render_line(&lines->lines[i], self, ctx, x, cur_y);


		if (!lines->lines[i].first && lines->lines[i].last->fmt & GP_MARKUP_STRIKE) {
			unsigned int line_h = gp_text_height(ctx->font)/25 + 1;

			gp_fill_rect_xywh(ctx->buf, x, cur_y, w, line_h, ctx->text_color);

			cur_y += ctx->padd;
		}
	}
}

enum keys {
	FMT,
	TEXT,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("fmt", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("text", GP_JSON_STR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_markup(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	gp_widget *ret = NULL;
	enum gp_markup_fmt fmt = GP_MARKUP_GFXPRIM;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case TEXT:
			ret = gp_widget_markup_new(val->val_str, fmt, 0);
		break;
		case FMT:
			if (ret)
				gp_json_warn(json, "Markup fmt must be defined before text");

			if (!strcmp(val->val_str, "plaintext"))
				fmt = GP_MARKUP_PLAINTEXT;
			else if (!strcmp(val->val_str, "gfxprim"))
				fmt = GP_MARKUP_GFXPRIM;
			else if (!strcmp(val->val_str, "html"))
				fmt = GP_MARKUP_HTML;
			else
				gp_json_warn(json, "Invalid markup fmt");
		break;
		}
	}

	if (!ret) {
		GP_WARN("Missing markup");
		return NULL;
	}

	return ret;
}

struct gp_widget_ops gp_widget_markup_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.from_json = json_to_markup,
	.id = "markup",
};

gp_widget *gp_widget_markup_new(const char *markup_str, enum gp_markup_fmt fmt, int flags)
{
	size_t payload_size = sizeof(struct gp_widget_markup);
	gp_widget *ret;

	gp_markup *markup = gp_markup_parse(fmt, markup_str, flags);
	if (!markup)
		return NULL;

	ret = gp_widget_new(GP_WIDGET_MARKUP, GP_WIDGET_CLASS_NONE, payload_size);
	if (!ret) {
		gp_markup_free(markup);
		return NULL;
	}

	ret->markup->markup = markup;

	return ret;
}

int gp_widget_markup_set(gp_widget *self, enum gp_markup_fmt fmt,
                         int flags, const char *markup_str)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_MARKUP, 1);

	gp_markup *markup = gp_markup_parse(fmt, markup_str, flags);
	if (!markup)
		return 1;

	gp_markup_free(self->markup->markup);

	self->markup->markup = markup;

	gp_widget_resize(self);
	gp_widget_redraw(self);

	return 0;
}
