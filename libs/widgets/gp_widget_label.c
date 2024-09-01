//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <utils/gp_vec_str.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

struct label_payload {
	/* Text buffer */
	char *text;
	/* Text format */
	char *text_fmt;
	/* widget size hints */
	const char *set;
	uint8_t width;
	/* attributes */
	uint8_t padd;
	enum gp_widget_label_flags flags;
	/* colors */
	uint8_t text_color;
	uint8_t bg_color;
	/* text attributes */
	gp_widget_tattr tattr;
};

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(label->tattr, ctx);
	unsigned int max_width;

	if (label->width) {
		max_width = gp_text_max_width_chars(font, label->set, label->width);
	} else {
		max_width = GP_MAX(gp_text_wbbox(font, label->text_fmt),
		                   gp_text_wbbox(font, label->text));
	}

	if ((label->flags & GP_WIDGET_LABEL_FRAME) && !label->padd)
		max_width += 2 * ctx->padd;

	return max_width + 2 * ctx->padd * label->padd;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(label->tattr, ctx);

	return 2 * ctx->padd + gp_text_ascent(font);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	gp_pixel bg_color = gp_widgets_color(ctx, label->bg_color);
	gp_pixel text_color = gp_widgets_color(ctx, label->text_color);

	(void) flags;

	if (gp_widget_is_disabled(self, flags))
		text_color = ctx->col_disabled;

	gp_widget_ops_blit(ctx, x, y, w, h);

	const gp_text_style *font = gp_widget_tattr_font(label->tattr, ctx);

	if (label->flags & GP_WIDGET_LABEL_FRAME) {
		gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color,
		                   bg_color, ctx->text_color);

	} else {
		gp_fill_rect_xywh(ctx->buf, x, y, w, h, bg_color);
	}

	if ((label->flags & GP_WIDGET_LABEL_FRAME) && !label->padd) {
		x += ctx->padd;
		w -= 2 * ctx->padd;
	}

	if (label->padd) {
		x += ctx->padd * label->padd;
		w -= 2 * ctx->padd * label->padd;
	}

	int align = gp_widget_tattr_halign(label->tattr);

	if (!align)
		align = GP_ALIGN_RIGHT;

	gp_text_fit(ctx->buf, font, x, y + ctx->padd, w,
	            align|GP_VALIGN_BELOW,
	            text_color, bg_color, label->text);
}

enum keys {
	BG_COLOR,
	FMT,
	FRAME,
	PADD,
	REVERSE_COLORS,
	TATTR,
	TEXT,
	TEXT_COLOR,
	WIDTH
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("bg_color", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("fmt", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("frame", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR("padd", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("reverse_colors", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR("tattr", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("text", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("text_color", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("width", GP_JSON_INT),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_label(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	char *label_text = NULL, *fmt = NULL;
	int width = 0;
	enum gp_widget_label_flags flags = 0;
	int bg_color = GP_WIDGETS_COL_BG;
	int text_color = GP_WIDGETS_COL_TEXT;
	int padd = 0;
	int reverse_colors = 0;
	gp_widget_tattr attr = 0;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case BG_COLOR:
			bg_color = gp_widgets_color_name_idx(val->val_str);
			if (bg_color < 0) {
				gp_json_warn(json, "Invalid background color name '%s'", val->val_str);
				bg_color = GP_WIDGETS_COL_BG;
			}
		break;
		case FMT:
			fmt = strdup(val->val_str);
		break;
		case FRAME:
			if (val->val_bool)
				flags |= GP_WIDGET_LABEL_FRAME;
		break;
		case REVERSE_COLORS:
			reverse_colors = val->val_bool;
		break;
		case PADD:
			if (padd < 0 || padd > UINT8_MAX)
				gp_json_warn(json, "Padding out of range %i", padd);
			else
				padd = val->val_int;
		break;
		case TATTR:
			if (gp_widget_tattr_parse(val->val_str, &attr, GP_TATTR_FONT | GP_TATTR_HALIGN))
				gp_json_warn(json, "Invalid text attribute '%s'", val->val_str);
		break;
		case TEXT:
			label_text = strdup(val->val_str);
		break;
		case TEXT_COLOR:
			text_color = gp_widgets_color_name_idx(val->val_str);
			if (text_color < 0) {
				gp_json_warn(json, "Invalid text color name '%s'", val->val_str);
				text_color = GP_WIDGETS_COL_TEXT;
			}
		break;
		case WIDTH:
			width = val->val_int;
		break;
		}
	}

	if (reverse_colors)
		GP_SWAP(bg_color, text_color);

	gp_widget *ret = gp_widget_label_new(label_text, attr, width);
	if (!ret)
		return NULL;

	struct label_payload *label = GP_WIDGET_PAYLOAD(ret);

	label->flags = flags;
	label->padd = padd;
	label->bg_color = bg_color;
	label->text_color = text_color;
	label->text_fmt = fmt;

	free(label_text);

	return ret;
}

static void free_(gp_widget *self)
{
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	gp_vec_free(label->text);
	free(label->text_fmt);
}

struct gp_widget_ops gp_widget_label_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.free = free_,
	.from_json = json_to_label,
	.id = "label",
};

static void redraw_resize(gp_widget *self)
{
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	gp_widget_redraw(self);

	if (label->width)
		return;

	gp_widget_resize(self);
}

void gp_widget_label_set(gp_widget *self, const char *text)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, );
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	GP_DEBUG(3, "Setting widget label (%p) text '%s' -> '%s'",
		 self, label->text, text);

	label->text = gp_vec_printf(label->text, "%s", text);

	redraw_resize(self);
}

const char *gp_widget_label_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, NULL);
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	return label->text;
}

void gp_widget_label_fmt_set(gp_widget *self, const char *text_fmt)
{
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	free(label->text_fmt);
	label->text_fmt = strdup(text_fmt);
}

void gp_widget_label_fmt_var_set(gp_widget *self, const char *fmt, ...)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, );
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	if (!label->text_fmt) {
		GP_WARN("Label (%p) format not set1", self);
		return;
	}

	GP_DEBUG(3, "Setting widget label (%p) from format '%s'",
		 self, label->text_fmt);

	va_list va, vac;
	size_t len = strlen(label->text_fmt);

	va_start(va, fmt);
	va_copy(vac, va);
	len += vsnprintf(NULL, 0, fmt, va);
	va_end(va);

	char *text = gp_vec_resize(label->text, len);
	if (!text)
		return;

	label->text = text;

	char *src = label->text_fmt;
	char *dst = label->text;
	char prev = 0;

	while (*src) {
		if (*src == '{' && prev != '\\') {
			dst += vsprintf(dst, fmt, vac);

			while (*src && (*src != '}' && prev != '\\'))
				prev = *(src++);

			if (*src)
				src++;
		}

		prev = *(dst++) = *(src++);
	}

	*dst = 0;

	va_end(vac);

	redraw_resize(self);
}

void gp_widget_label_append(gp_widget *self, const char *text)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, );
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	GP_DEBUG(3, "Appending to label widget (%p) '%s' += '%s'",
		 self, label->text, text);

	char *new_text = gp_vec_str_append(label->text, text);
	if (!new_text)
		return;

	label->text = new_text;

	redraw_resize(self);
}

gp_widget *gp_widget_label_new(const char *text, gp_widget_tattr tattr, unsigned int width)
{
	gp_widget *ret;

	ret = gp_widget_new(GP_WIDGET_LABEL, GP_WIDGET_CLASS_NONE, sizeof(struct label_payload));
	if (!ret)
		return NULL;

	ret->no_shrink = 1;

	struct label_payload *label = GP_WIDGET_PAYLOAD(ret);

	if (text)
		label->text = gp_vec_strdup(text);
	else
		label->text = gp_vec_str_new();

	label->tattr = tattr;
	label->width = width;

	label->bg_color = GP_WIDGETS_COL_BG;
	label->text_color = GP_WIDGETS_COL_TEXT;

	return ret;
}

static char *valloc_printf(const char *fmt, va_list ap)
{
	va_list ac;
	size_t len;

	va_copy(ac, ap);
	len = vsnprintf(NULL, 0, fmt, ac);
	va_end(ac);

	char *buf = malloc(len+1);
	if (!buf) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	vsnprintf(buf, len+1, fmt, ap);

	return buf;
}

gp_widget *gp_widget_label_printf_new(gp_widget_tattr tattr, const char *fmt, ...)
{
	va_list ap;
	char *buf;

	va_start(ap, fmt);
	buf = valloc_printf(fmt, ap);
	va_end(ap);

	gp_widget *ret = gp_widget_label_new(buf, tattr, 0);

	free(buf);

	return ret;
}

int gp_widget_label_printf(gp_widget *self, const char *fmt, ...)
{
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);
	va_list ap;

	va_start(ap, fmt);
	label->text = gp_vec_vprintf(label->text, fmt, ap);
	va_end(ap);

	redraw_resize(self);

	return 0;
}

void gp_widget_label_vprintf(gp_widget *self, const char *fmt, va_list ap)
{
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	label->text = gp_vec_vprintf(label->text, fmt, ap);
	redraw_resize(self);
}

void gp_widget_label_tattr_set(gp_widget *self, gp_widget_tattr tattr)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, );
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	label->tattr = tattr;

	gp_widget_redraw(self);
}

gp_widget_tattr gp_widget_label_tattr_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, 0);
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	return label->tattr;
}

void gp_widget_label_width_set(gp_widget *self, unsigned int width)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, );
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	label->width = width;
	gp_widget_resize(self);
}

unsigned int gp_widget_label_width_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, 0);
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	return label->width;
}

void gp_widget_label_flags_set(gp_widget *self, enum gp_widget_label_flags flags)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, );
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	if (flags & ~GP_WIDGET_LABEL_FRAME) {
		GP_WARN("Invalid label flags");
		return;
	}

	label->flags = flags;

	gp_widget_resize(self);
}

enum gp_widget_label_flags gp_widget_label_flags_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, 0);
	struct label_payload *label = GP_WIDGET_PAYLOAD(self);

	return label->flags;
}
