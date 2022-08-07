//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <utils/gp_vec.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	const gp_text_style *font = gp_widget_tattr_font(self->log->tattr, ctx);

	return gp_text_avg_width(font, self->log->min_width) + 2 * ctx->padd;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	const gp_text_style *font = gp_widget_tattr_font(self->log->tattr, ctx);

	return self->log->min_lines * (ctx->padd + gp_text_ascent(font)) + ctx->padd;
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	(void) flags;

	struct gp_widget_log *log = self->log;

	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	const gp_text_style *font = gp_widget_tattr_font(self->log->tattr, ctx);

	unsigned int line_h = gp_text_ascent(font) + ctx->padd;
	unsigned int line_w = w - 2 * ctx->padd;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, ctx->fg_color, ctx->text_color);

	/* Figure out how many lines we can fit into the box first */
	size_t box_lines = (h - ctx->padd) / line_h;
	size_t logs = gp_vec_len(log->logs);
	size_t cur_str = logs;
	size_t cur_line = 0;

	if (!logs)
		return;

	for (;;) {
		if (!cur_str)
			break;

		if (cur_line >= box_lines)
			break;

		cur_str--;

		const char *text = log->logs[cur_str];

		size_t str_width = gp_text_wbbox(font, text);
		size_t lines_per_str = GP_MAX((size_t)1, str_width / line_w + !!(str_width % line_w));

		cur_line += lines_per_str;
	}

	size_t render_line = GP_MIN(cur_line, box_lines);

	/* Then finally render it */
	x += ctx->padd;
	y += ctx->padd;

	for (;;) {
		const char *line = log->logs[cur_str];
		size_t line_len = strlen(line);

		for (;;) {
			if (!render_line)
				return;

			size_t chars = gp_text_fit_width(font, line, line_w);

			gp_print(ctx->buf, font, x, y,
			         GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
				 ctx->text_color, ctx->fg_color,
				 "%.*s", (int)chars, line);

			y += line_h;
			render_line--;

			line_len -= chars;
			line += chars;

			if (!line_len)
				break;
		}

		if (++cur_str >= logs)
			return;
	}
}

enum keys {
	MIN_LINES,
	MIN_WIDTH,
	TATTR,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("min_lines", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("min_width", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("tattr", GP_JSON_STR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_log(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	int width = 80;
	int lines = 25;
	gp_widget_tattr attr = 0;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case MIN_LINES:
			lines = val->val_int;
			if (lines <= 0) {
				gp_json_warn(json, "Invalid min lines %i", lines);
				return NULL;
			}
		break;
		case MIN_WIDTH:
			width = val->val_int;
			if (width <= 0) {
				gp_json_warn(json, "Invalid min width %i", lines);
				return NULL;
			}
		break;
		case TATTR:
			if (gp_widget_tattr_parse(val->val_str, &attr, GP_TATTR_FONT | GP_TATTR_HALIGN))
				gp_json_warn(json, "Invalid text attribute '%s'", val->val_str);
		break;
		}
	}

	gp_widget *ret = gp_widget_log_new(attr, width, lines);

	return ret;
}

static void free_(gp_widget *self)
{
	if (!self->log->logs)
		return;

	GP_VEC_FOREACH(self->log->logs, char *, log)
		free(*log);

	gp_vec_free(self->log->logs);
}

struct gp_widget_ops gp_widget_log_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.free = free_,
	.from_json = json_to_log,
	.id = "log",
};

void gp_widget_log_append(gp_widget *self, const char *text)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_LOG, );
	char *str;

	GP_DEBUG(3, "Appending to log widget (%p) '%s'", self, text);

	str = strdup(text);
	if (!GP_VEC_APPEND(self->log->logs, str)) {
		free(str);
		return;
	}

	gp_widget_redraw(self);
}

gp_widget *gp_widget_log_new(gp_widget_tattr tattr,
                             unsigned int min_width, unsigned int min_lines)
{
	gp_widget *ret;

	if (min_width <= 0 || min_lines <= 0) {
		GP_WARN("Invalid min_width or min_lines");
		return NULL;
	}

	ret = gp_widget_new(GP_WIDGET_LOG, GP_WIDGET_CLASS_NONE, sizeof(struct gp_widget_log));
	if (!ret)
		return NULL;

	ret->log->tattr = tattr;
	ret->log->min_width = min_width;
	ret->log->min_lines = min_lines;
	ret->log->logs = gp_vec_new(0, sizeof(char **));

	if (!ret->log->logs) {
		gp_widget_free(ret);
		return NULL;
	}

	return ret;
}
