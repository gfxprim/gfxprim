//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <utils/gp_vec.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

struct gp_widget_log {
	gp_widget_tattr tattr;
	unsigned int min_width;
	unsigned int min_lines;

	gp_cbuffer log;
	char **logs;
};

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_log *log = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(log->tattr, ctx);

	return gp_text_avg_width(font, log->min_width) + 2 * ctx->padd;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_log *log = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(log->tattr, ctx);

	return log->min_lines * (ctx->padd + gp_text_ascent(font)) + ctx->padd;
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	(void) flags;

	struct gp_widget_log *log = GP_WIDGET_PAYLOAD(self);

	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	const gp_text_style *font = gp_widget_tattr_font(log->tattr, ctx);

	unsigned int line_h = gp_text_ascent(font) + ctx->padd;
	unsigned int line_w = w - 2 * ctx->padd;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, ctx->fg_color, ctx->text_color);

	if (!gp_cbuffer_used(&log->log))
		return;

	/* Figure out how many lines we can fit into the box first */
	size_t box_lines = (h - ctx->padd) / line_h;
	size_t cur_line = 0;

	gp_cbuffer_iter iter;

	GP_CBUFFER_FOREACH_REV(&log->log, &iter) {
		if (cur_line >= box_lines)
			break;

		const char *text = log->logs[iter.idx];

		size_t str_width = gp_text_wbbox(font, text);
		size_t lines_per_str = GP_MAX((size_t)1, str_width / line_w + !!(str_width % line_w));

		cur_line += lines_per_str;
	}

	size_t render_lines = GP_MIN(cur_line, box_lines);
	size_t off_lines = cur_line - render_lines;

	/* Then finally render it */
	x += ctx->padd;
	y += ctx->padd;

	size_t count = iter.cnt;
	size_t first = gp_cbuffer_used(&log->log) - count;

	GP_CBUFFER_FORRANGE(&log->log, &iter, first, count) {
		const char *line = log->logs[iter.idx];
		size_t line_len = strlen(line);

		for (;;) {
			if (!render_lines)
				return;

			size_t chars = gp_text_fit_width(font, line, line_w);

			if (off_lines) {
				line_len -= chars;
				line += chars;
				off_lines--;
				continue;
			}

			gp_print(ctx->buf, font, x, y,
			         GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
				 ctx->text_color, ctx->fg_color,
				 "%.*s", (int)chars, line);

			y += line_h;
			render_lines--;

			line_len -= chars;
			line += chars;

			if (!line_len)
				break;
		}
	}
}

enum keys {
	MAX_LOGS,
	MIN_LINES,
	MIN_WIDTH,
	TATTR,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(MAX_LOGS, "max_logs", GP_JSON_INT),
	GP_JSON_OBJ_ATTR_IDX(MIN_LINES, "min_lines", GP_JSON_INT),
	GP_JSON_OBJ_ATTR_IDX(MIN_WIDTH, "min_width", GP_JSON_INT),
	GP_JSON_OBJ_ATTR_IDX(TATTR, "tattr", GP_JSON_STR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_log(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	int width = 80;
	int lines = 25;
	int max_logs = 0;
	gp_widget_tattr attr = 0;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case MAX_LOGS:
			max_logs = val->val_int;
			if (lines <= 0) {
				gp_json_warn(json, "Invalid max logs %i", max_logs);
				return NULL;
			}
		break;
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

	gp_widget *ret = gp_widget_log_new(attr, width, lines, max_logs);

	return ret;
}

static void free_(gp_widget *self)
{
	struct gp_widget_log *log = GP_WIDGET_PAYLOAD(self);
	gp_cbuffer_iter iter;

	if (!log->logs)
		return;

	GP_CBUFFER_FOREACH(&log->log, &iter)
		free(log->logs[iter.idx]);

	free(log->logs);
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
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LOG, );
	struct gp_widget_log *log = GP_WIDGET_PAYLOAD(self);
	char *str;

	GP_DEBUG(3, "Appending to log widget (%p) '%s'", self, text);

	str = strdup(text);
	if (!str) {
		GP_DEBUG(3, "Malloc failed :(");
		return;
	}

	size_t idx = gp_cbuffer_append(&log->log);

	free(log->logs[idx]);
	log->logs[idx] = str;

	gp_widget_redraw(self);
}

gp_widget *gp_widget_log_new(gp_widget_tattr tattr,
                             unsigned int min_width, unsigned int min_lines,
			     size_t max_logs)
{
	gp_widget *ret;

	if (min_width <= 0 || min_lines <= 0) {
		GP_WARN("Invalid min_width or min_lines");
		return NULL;
	}

	if (!max_logs) {
		max_logs = (size_t)10 * min_lines;
		GP_DEBUG(1, "Defaulting to max logs = 10 * min_lines = %zu", max_logs);
	}

	ret = gp_widget_new(GP_WIDGET_LOG, GP_WIDGET_CLASS_NONE, sizeof(struct gp_widget_log));
	if (!ret)
		return NULL;

	struct gp_widget_log *log = GP_WIDGET_PAYLOAD(ret);

	log->tattr = tattr;
	log->min_width = min_width;
	log->min_lines = min_lines;
	log->logs = malloc(sizeof(char **) * max_logs);

	if (!log->logs) {
		gp_widget_free(ret);
		return NULL;
	}

	memset(log->logs, 0, sizeof(char **) * max_logs);
	gp_cbuffer_init(&log->log, max_logs);

	return ret;
}
