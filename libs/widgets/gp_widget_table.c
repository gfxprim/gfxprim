//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <gp_widgets.h>
#include <gp_widget_ops.h>
#include <gp_widget_render.h>

static unsigned int header_min_w(gp_widget_table *tbl,
                                 const gp_widget_render_ctx *ctx,
                                 unsigned int col)
{
	const char *text = tbl->headers[col].text;
	unsigned int text_size = gp_text_width(ctx->font_bold, text);

	if (tbl->headers[col].sortable)
		text_size += ctx->padd + gp_text_ascent(ctx->font);

	return text_size;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_table *tbl = self->tbl;
	unsigned int i, sum_cols_w = 0;

	if (tbl->headers) {
		for (i = 0; i < tbl->cols; i++)
			tbl->cols_w[i] = header_min_w(tbl, ctx, i);
	}

	for (i = 0; i < tbl->cols; i++) {
		unsigned int col_size;
		col_size = gp_text_max_width(ctx->font, tbl->col_min_sizes[i]);
		tbl->cols_w[i] = GP_MAX(tbl->cols_w[i], col_size);
		sum_cols_w += tbl->cols_w[i];
	}

	return sum_cols_w + (2 * tbl->cols) * ctx->padd;
}

static unsigned int header_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int text_a = gp_text_ascent(ctx->font);

	if (!self->tbl->headers)
		return 0;

	return text_a + 2 * ctx->padd;
}

static unsigned int row_h(const gp_widget_render_ctx *ctx)
{
	unsigned int text_a = gp_text_ascent(ctx->font);

	return text_a + ctx->padd;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int h = row_h(ctx) * self->tbl->min_rows;

	if (self->tbl->headers)
		h += header_h(self, ctx);

	return h;
}

static unsigned int display_rows(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int text_a = gp_text_ascent(ctx->font);
	unsigned int header = header_h(self, ctx);

	return (self->h - header) / (text_a + ctx->padd);
}

static void distribute_size(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	gp_widget_table *tbl = self->tbl;
	unsigned int i, sum_cols_w = 0, sum_fills = 0;

	(void)new_wh;

	for (i = 0; i < tbl->cols; i++) {
		sum_cols_w += tbl->cols_w[i];
		sum_fills += tbl->col_fills[i];
	}

	if (!sum_fills)
		return;

	unsigned int table_w = sum_cols_w + (2 * tbl->cols) * ctx->padd;
	unsigned int diff = self->w - table_w;

	for (i = 0; i < tbl->cols; i++)
		tbl->cols_w[i] += tbl->col_fills[i] * (diff/sum_fills);
}

static void header_render(gp_widget *self, gp_coord x, gp_coord y,
                          const gp_widget_render_ctx *ctx)
{
	gp_widget_table *tbl = self->tbl;
	const gp_widget_table_header *headers = tbl->headers;
	unsigned int text_a = gp_text_ascent(ctx->font);
	unsigned int cx = x + ctx->padd;
	unsigned int cy = y + ctx->padd;
	unsigned int i;

	for (i = 0; i < tbl->cols; i++) {
		if (tbl->headers[i].sortable) {
			gp_size symbol_size = text_a/1.5;
			gp_size sx = cx + tbl->cols_w[i] - ctx->padd;
			gp_size sy = cy + text_a/2;

			if (i == tbl->sorted_by_col) {
				if (tbl->sorted_desc)
					gp_triangle_down(ctx->buf, sx, sy, symbol_size, ctx->text_color);
				else
					gp_triangle_up(ctx->buf, sx, sy, symbol_size, ctx->text_color);
			} else {
				gp_triangle_updown(ctx->buf, sx, sy, symbol_size, ctx->text_color);
			}
		}

		gp_print(ctx->buf, ctx->font_bold, cx, cy,
			GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
			ctx->text_color, ctx->bg_color, "%s", headers[i].text);

		cx += tbl->cols_w[i] + ctx->padd;

		if (i < tbl->cols - 1) {
			gp_vline_xyh(ctx->buf, cx, y+1,
			            text_a + 2 * ctx->padd-1, ctx->bg_color);
		}

		cx += ctx->padd;
	}

	cy += text_a + ctx->padd;

	gp_pixel color = self->focused ? ctx->sel_color : ctx->text_color;

	gp_hline_xyw(ctx->buf, x, cy, self->w, color);
}

static void align_text(gp_pixmap *buf, gp_widget_table *tbl,
                       const gp_widget_render_ctx *ctx,
		       unsigned int x, unsigned int y,
		       unsigned int col, gp_pixel bg, const char *str)
{
	gp_text_fit(buf, ctx->font, x, y, tbl->cols_w[col],
	           GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
	           ctx->text_color, bg, str);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct gp_widget_table *tbl = self->tbl;
	unsigned int text_a = gp_text_ascent(ctx->font);
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	unsigned int cy = y;
	unsigned int i, j;

	(void)flags;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_pixel color = self->focused ? ctx->sel_color : ctx->text_color;
	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, ctx->fg_color, color);

	if (tbl->headers) {
		header_render(self, x, y, ctx);
		cy += header_h(self, ctx);
	}

	tbl->row(self, GP_TABLE_ROW_RESET, 0);
	tbl->row(self, GP_TABLE_ROW_ADVANCE, tbl->start_row);

	unsigned int cur_row = tbl->start_row;
	unsigned int rows = display_rows(self, ctx);

	unsigned int cx = x + ctx->padd;

	for (j = 0; j < tbl->cols-1; j++) {
		cx += tbl->cols_w[j] + ctx->padd;
		gp_vline_xyy(ctx->buf, cx, cy+1, self->y + offset->y + self->h - 2, ctx->bg_color);
		cx += ctx->padd;
	}

	cy += ctx->padd/2;
	for (i = 0; i < rows; i++) {
		cx = x + ctx->padd;
		gp_pixel bg_col = ctx->fg_color;

		if (tbl->row_focused && cur_row == tbl->focused_row) {
			bg_col = self->focused ? ctx->sel_color : ctx->bg_color;

			gp_fill_rect_xywh(ctx->buf, x+1, cy - ctx->padd/2+1,
					self->w - 2,
					text_a + ctx->padd-1, bg_col);
		}

		for (j = 0; j < tbl->cols; j++) {
			const char *str = tbl->get(self, j);

			align_text(ctx->buf, tbl, ctx, cx, cy, j, bg_col, str);

			cx += tbl->cols_w[j] + ctx->padd;

		//	if (j < tbl->cols - 1) {
		//		gp_vline_xyh(ctx->buf, cx, cy,
		//			    text_a, ctx->text_color);
		//	}

			cx += ctx->padd;
		}

		cy += text_a + ctx->padd;

		tbl->row(self, GP_TABLE_ROW_ADVANCE, 1);
		cur_row++;

		gp_hline_xyw(ctx->buf, x+1, cy - ctx->padd/2, w-2, ctx->bg_color);
	}

	while (tbl->row(self, GP_TABLE_ROW_ADVANCE, 1))
		cur_row++;

	tbl->last_max_row = cur_row;
}

static void fix_focused_row(gp_widget_table *tbl)
{
	if (tbl->focused_row >= tbl->last_max_row)
		tbl->focused_row = tbl->last_max_row - 1;
}

static int move_down(gp_widget *self, const gp_widget_render_ctx *ctx,
                     unsigned int rows)
{
	gp_widget_table *tbl = self->tbl;

	if (!tbl->row_focused) {
		tbl->row_focused = 1;
		tbl->focused_row = tbl->start_row;

		fix_focused_row(tbl);

		goto redraw;
	}

	if (tbl->focused_row < tbl->last_max_row) {
		tbl->focused_row += rows;

		fix_focused_row(tbl);

		goto redraw;
	}

	return 0;

redraw:
	rows = display_rows(self, ctx);

	if (tbl->focused_row > tbl->start_row + rows)
		tbl->start_row = tbl->focused_row - rows + 1;

	gp_widget_redraw(self);
	return 1;
}

static int move_up(gp_widget *self, const gp_widget_render_ctx *ctx, unsigned int rows)
{
	gp_widget_table *tbl = self->tbl;

	if (!tbl->row_focused) {
		tbl->row_focused = 1;
		tbl->focused_row = tbl->start_row + display_rows(self, ctx) - 1;

		goto redraw;
	}

	if (tbl->focused_row > 0) {
		if (tbl->focused_row > rows)
			tbl->focused_row -= rows;
		else
			tbl->focused_row = 0;

		goto redraw;
	}

	return 0;

redraw:
	if (tbl->focused_row < tbl->start_row)
		tbl->start_row = tbl->focused_row;

	gp_widget_redraw(self);
	return 1;
}

static int header_click(gp_widget *self, const gp_widget_render_ctx *ctx, unsigned int x)
{
	gp_widget_table *tbl = self->tbl;
	unsigned int i, cx = 0;

	//TODO: inverval division?
	for (i = 0; i < tbl->cols-1; i++) {
		cx += tbl->cols_w[i] + 2 * ctx->padd;

		if (x <= cx)
			break;
	}

	if (!tbl->headers[i].sortable)
		return 0;

	if (!tbl->sort) {
		GP_BUG("No sort fuction defined!");
		return 1;
	}

	if (tbl->sorted_by_col == i)
		tbl->sorted_desc = !tbl->sorted_desc;
	else
		tbl->sorted_by_col = i;

	tbl->sort(self, tbl->sorted_by_col, tbl->sorted_desc);

	gp_widget_redraw(self);
	return 1;
}

static int row_click(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	gp_widget_table *tbl = self->tbl;
	unsigned int row = ev->cursor_y - header_h(self, ctx);

	row /= row_h(ctx) + tbl->start_row;
	tbl->focused_row = row;

	if (!tbl->row_focused)
		tbl->row_focused = 1;

	gp_widget_redraw(self);
	return 1;
}

static int click(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	if (ev->cursor_y <= header_h(self, ctx))
		return header_click(self, ctx, ev->cursor_x);

	return row_click(self, ctx, ev);
}

static int enter(gp_widget *self)
{
	gp_widget_table *tbl = self->tbl;

	if (!tbl->row_focused)
		return 0;

	gp_widget_send_widget_event(self, 0);

	return 1;
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		switch (ev->val) {
		case GP_KEY_DOWN:
			if (gp_event_get_key(ev, GP_KEY_LEFT_SHIFT))
				return move_down(self, ctx, 10);

			return move_down(self, ctx, 1);
		break;
		case GP_KEY_UP:
			if (gp_event_get_key(ev, GP_KEY_LEFT_SHIFT))
				return move_up(self, ctx, 10);

			return move_up(self, ctx, 1);
		break;
		//TODO: Better page up/down
		case GP_KEY_PAGE_UP:
			return move_up(self, ctx, 10);
		case GP_KEY_PAGE_DOWN:
			return move_down(self, ctx, 10);
		case GP_BTN_LEFT:
			return click(self, ctx, ev);
		case GP_KEY_ENTER:
			return enter(self);
		}
	}

	return 0;
}

static gp_widget_table_header *parse_header(json_object *json, int *cols)
{
	gp_widget_table_header *header;
	int i;

	if (!json)
		return NULL;

	if (!json_object_is_type(json, json_type_array)) {
		GP_WARN("Table header must be array!");
		return NULL;
	}

	if (*cols == -1) {
		*cols = json_object_array_length(json);
	} else if (json_object_array_length(json) != (size_t)(*cols)) {
		GP_WARN("Table header is not equal to number of columns!");
		return NULL;
	}

	header = malloc(sizeof(*header) * (*cols));
	if (!header)
		return NULL;

	for (i = 0; i < *cols; i++) {
		json_object *elem = json_object_array_get_idx(json, i);
		json_object *tmp;

		if (!elem) {
			GP_WARN("Table header parse error!");
			goto err;
		}

		tmp = json_object_object_get(elem, "label");
		if (!tmp) {
			GP_WARN("Table header %i is missing label", i);
			goto err;
		}

		header[i].text = strdup(json_object_get_string(tmp));

		if ((tmp = json_object_object_get(elem, "sortable")))
			header[i].sortable = json_object_get_boolean(tmp);

		header[i].text_align = 0;
	}

	return header;
err:
	free(header);
	return NULL;
}

static gp_widget *json_to_table(json_object *json, void **uids)
{
	int cols = -1, min_rows = -1;
	void *set_row = NULL, *get_elem = NULL, *sort = NULL;
	json_object *header = NULL;
	gp_widget_table_header *table_header;

	(void)uids;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "cols"))
			cols = json_object_get_int(val);
		else if (!strcmp(key, "min_rows"))
			min_rows = json_object_get_int(val);
		else if (!strcmp(key, "set_row"))
			set_row = gp_widget_callback_addr(json_object_get_string(val));
		else if (!strcmp(key, "get_elem"))
			get_elem = gp_widget_callback_addr(json_object_get_string(val));
		else if (!strcmp(key, "sort"))
			sort = gp_widget_callback_addr(json_object_get_string(val));
		else if (!strcmp(key, "header"))
			header = val;
		else
			GP_WARN("Invalid table key '%s'", key);
	}

	table_header = parse_header(header, &cols);

	if (cols < 0) {
		GP_WARN("Invalid or missing cols");
		return NULL;
	}

	if (min_rows < 0) {
		GP_WARN("Invalid or missing min_rows");
		return NULL;
	}

	if (!set_row) {
		GP_WARN("Invalid or missing set_row callback");
		return NULL;
	}

	if (!get_elem) {
		GP_WARN("Invalid or missing get_elem callback");
		return NULL;
	}

	gp_widget *table = gp_widget_table_new(cols, min_rows, table_header, set_row, get_elem);

	//TODO: Free header
	if (!table)
		return NULL;

	table->tbl->sort = sort;

	return table;
}

struct gp_widget_ops gp_widget_table_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.distribute_size = distribute_size,
	.render = render,
	.event = event,
	.from_json = json_to_table,
	.id = "table",
};

gp_widget *gp_widget_table_new(unsigned int cols, unsigned int min_rows,
                               const gp_widget_table_header *headers,
                               int (*row)(struct gp_widget *self,
                                          int op, unsigned int pos),
                               const char *(get)(struct gp_widget *self,
                                                 unsigned int col))
{
	gp_widget *ret;
	size_t size = sizeof(struct gp_widget_table);

	size += 2 * cols * sizeof(unsigned int);
	size += cols;

	ret = gp_widget_new(GP_WIDGET_TABLE, size);
	if (!ret)
		return NULL;

	ret->tbl->cols = cols;
	ret->tbl->min_rows = min_rows;
	ret->tbl->start_row = 0;
	ret->tbl->cols_w = (void*)ret->tbl->buf;
	ret->tbl->col_min_sizes = (void*)(ret->tbl->buf + cols * sizeof(unsigned int));
	ret->tbl->col_fills = (void*)(ret->tbl->buf + 2 * cols * sizeof(unsigned int));
	ret->tbl->headers = headers;

	ret->tbl->get = get;
	ret->tbl->row = row;

	return ret;
}

void gp_widget_table_refresh(gp_widget *self)
{
	gp_widget_redraw(self);
}
