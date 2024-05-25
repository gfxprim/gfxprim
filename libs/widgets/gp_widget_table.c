//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

static int col_is_sortable(gp_widget_table *tbl, unsigned int head_idx)
{
	gp_widget_table_col_desc *col = tbl->header[head_idx].col_desc;

	if (!col)
		return 0;

	return col->sortable;
}

static void sort_by_col(gp_widget *self, int desc, unsigned int head_idx)
{
	gp_widget_table_col_desc *col = self->tbl->header[head_idx].col_desc;

	if (!col)
		return;

	if (!col->sortable)
		return;

	self->tbl->col_ops.sort(self, desc, col->idx);
}

static inline int get_cell(gp_widget *self, gp_widget_table_cell *ret, unsigned int head_idx)
{
	gp_widget_table_col_desc *col = self->tbl->header[head_idx].col_desc;

	if (!col)
		return 0;

	return self->tbl->col_ops.get_cell(self, ret, col->idx);
}

static inline int seek_row(gp_widget *self, int op, unsigned int pos)
{
	return self->tbl->col_ops.seek_row(self, op, pos);
}

static unsigned int header_min_w(gp_widget_table *tbl,
                                 const gp_widget_render_ctx *ctx,
                                 unsigned int col)
{
	const char *label = tbl->header[col].label;
	const gp_text_style *font = gp_widget_tattr_font(tbl->header[col].tattr, ctx);
	unsigned int text_size = 0;

	if (label)
		text_size += gp_text_wbbox(font, label);

	if (col_is_sortable(tbl, col))
		text_size += ctx->padd + gp_text_ascent(ctx->font);

	return text_size;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_table *tbl = self->tbl;
	unsigned int i, sum_cols_w = 0;

	if (tbl->header) {
		for (i = 0; i < tbl->cols; i++)
			tbl->cols_w[i].min_size = header_min_w(tbl, ctx, i);
	}

	for (i = 0; i < tbl->cols; i++) {
		unsigned int col_size;

		col_size = gp_text_max_width(ctx->font, tbl->header[i].col_min_size);

		if (tbl->header)
			col_size = GP_MAX(col_size, header_min_w(tbl, ctx, i));

		tbl->cols_w[i].min_size = col_size;
		sum_cols_w += col_size;
	}

	return sum_cols_w + (2 * tbl->cols) * ctx->padd;
}

static unsigned int header_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	//TODO: Proper font handling!
	unsigned int text_a = gp_text_ascent(ctx->font);

	if (!self->tbl->needs_header)
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

	if (self->tbl->header)
		h += header_h(self, ctx);

	return h;
}

static unsigned int display_rows(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int text_a = gp_text_ascent(ctx->font);
	unsigned int header = header_h(self, ctx);

	if (header > self->h)
		return 0;

	return (self->h - header) / (text_a + ctx->padd);
}

static void distribute_w(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	gp_widget_table *tbl = self->tbl;
	unsigned int i, sum_cols_w = 0, sum_fills = 0;

	(void)new_wh;

	for (i = 0; i < tbl->cols; i++) {
		sum_cols_w += tbl->cols_w[i].min_size;
		sum_fills += tbl->header[i].col_fill;
		tbl->cols_w[i].size = tbl->cols_w[i].min_size;
	}

	if (!sum_fills)
		return;

	unsigned int table_w = sum_cols_w + (2 * tbl->cols) * ctx->padd;
	unsigned int diff = self->w - table_w;

	for (i = 0; i < tbl->cols; i++)
		tbl->cols_w[i].size += tbl->header[i].col_fill * (diff/sum_fills);
}

static unsigned int header_render(gp_widget *self, gp_coord x, gp_coord y,
                                  const gp_widget_render_ctx *ctx)
{
	gp_widget_table *tbl = self->tbl;
	const gp_widget_table_header *header = tbl->header;
	unsigned int text_a = gp_text_ascent(ctx->font);
	unsigned int cx = x + ctx->padd;
	unsigned int cy = y + ctx->padd;
	unsigned int i;

	if (!self->tbl->needs_header)
		return 0;

	for (i = 0; i < tbl->cols; i++) {
		unsigned int ex = cx + tbl->cols_w[i].size;

		if (col_is_sortable(tbl, i)) {
			gp_size sym_size = text_a/3;
			gp_size sx = cx + tbl->cols_w[i].size - ctx->padd;
			gp_size sy = cy + text_a/2;

			if (i == tbl->sorted_by_col) {
				if (tbl->sorted_desc)
					gp_symbol(ctx->buf, sx, sy, sym_size, sym_size, GP_TRIANGLE_DOWN, ctx->text_color);
				else
					gp_symbol(ctx->buf, sx, sy, sym_size, sym_size, GP_TRIANGLE_UP, ctx->text_color);
			} else {
				gp_symbol(ctx->buf, sx, sy - 2*(sym_size/2), 2*(sym_size/2), sym_size/2, GP_TRIANGLE_UP, ctx->text_color);
				gp_symbol(ctx->buf, sx, sy + 2*(sym_size/2), 2*(sym_size/2), sym_size/2, GP_TRIANGLE_DOWN, ctx->text_color);
			}

			ex -= gp_text_ascent(ctx->font) + ctx->padd;
		}

		if (header[i].label) {
			const gp_text_style *font = gp_widget_tattr_font(tbl->header[i].tattr, ctx);
			int halign = gp_widget_tattr_halign(tbl->header[i].tattr);

			if (!halign)
				halign = GP_ALIGN_LEFT;

			gp_text_xxy(ctx->buf, font, cx, ex, cy,
			            halign | GP_VALIGN_BELOW, ctx->text_color, ctx->bg_color, header[i].label);
		}

		cx += tbl->cols_w[i].size + ctx->padd;

		if (i < tbl->cols - 1) {
			gp_vline_xyh(ctx->buf, cx, y+1,
			            text_a + 2 * ctx->padd-1, ctx->bg_color);
		}

		cx += ctx->padd;
	}

	cy += text_a + ctx->padd;

	gp_pixel color = self->focused ? ctx->sel_color : ctx->text_color;

	gp_hline_xyw(ctx->buf, x, cy, self->w, color);

	return header_h(self, ctx);
}

static unsigned int last_row(gp_widget *self)
{
	int ret;

	ret = seek_row(self, GP_TABLE_ROW_MAX, 0);
	if (ret >= 0)
		return ret;

	ret = 0;

	if (!seek_row(self, GP_TABLE_ROW_RESET, 0))
		return 0;

	while (seek_row(self, GP_TABLE_ROW_ADVANCE, 1))
		ret++;

	return ret + 1;
}

static void fix_selected_row(gp_widget_table *tbl)
{
	if (tbl->selected_row >= tbl->last_rows)
		tbl->selected_row = tbl->last_rows - 1;
}

static void render_cell(gp_widget_table_cell *cell, const gp_widget_render_ctx *ctx,
                        gp_coord x, gp_coord y, gp_size w,
                        gp_pixel text_col, gp_pixel bg_col)
{
	const gp_text_style *font = gp_widget_tattr_font(cell->tattr & ~GP_TATTR_LARGE, ctx);
	int halign = gp_widget_tattr_halign(cell->tattr);

	if (!halign)
		halign = GP_ALIGN_LEFT;

	gp_text_fit(ctx->buf, font, x, y, w,
	            halign|GP_VALIGN_BASELINE, text_col, bg_col, cell->text);
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

	cy += header_render(self, x, y, ctx);

	tbl->last_rows = last_row(self);
	fix_selected_row(tbl);

	if (tbl->start_row > tbl->last_rows)
		tbl->start_row = 0;

	seek_row(self, GP_TABLE_ROW_RESET, 0);
	seek_row(self, GP_TABLE_ROW_ADVANCE, tbl->start_row);

	unsigned int cur_row = tbl->start_row;
	unsigned int rows = display_rows(self, ctx);

	unsigned int cx = x + ctx->padd;

	for (j = 0; j < tbl->cols-1; j++) {
		cx += tbl->cols_w[j].size + ctx->padd;
		gp_vline_xyy(ctx->buf, cx, cy+1, self->y + offset->y + self->h - 2, ctx->bg_color);
		cx += ctx->padd;
	}

	cy += ctx->padd/2;
	for (i = 0; i < rows; i++) {
		cx = x + ctx->padd;
		gp_pixel bg_col = ctx->fg_color;
		gp_pixel text_col = ctx->text_color;

		gp_coord ix = x+ctx->fr_thick;
		gp_coord iw = self->w - 2*ctx->fr_thick;

		if (tbl->row_selected && cur_row == tbl->selected_row) {
			bg_col = gp_widget_frame_color(self, ctx, flags);

			int is_1bpp = gp_pixel_size(ctx->pixel_type) == 1;

			if (is_1bpp && self->focused)
				text_col = ctx->fg_color;

			if (is_1bpp && !self->focused) {
				gp_rect_xywh(ctx->buf, ix, cy - ctx->padd/2+1,
				             iw, text_a + ctx->padd-1, bg_col);
			} else {
				bg_col = self->focused ? ctx->sel_color : ctx->bg_color;

				gp_fill_rect_xywh(ctx->buf, ix, cy - ctx->padd/2+1,
				                  iw, text_a + ctx->padd-1, bg_col);
			}

		}

		if (cur_row < tbl->last_rows) {
			for (j = 0; j < tbl->cols; j++) {
				gp_widget_table_cell cell = {};

				if (get_cell(self, &cell, j)) {
					unsigned int by = cy + text_a - ctx->padd/4;
					render_cell(&cell, ctx, cx, by, tbl->cols_w[j].size, text_col, bg_col);
				}

				cx += tbl->cols_w[j].size + ctx->padd;

			//	if (j < tbl->cols - 1) {
			//		gp_vline_xyh(ctx->buf, cx, cy,
			//			    text_a, ctx->text_color);
			//	}

				cx += ctx->padd;
			}
		}

		cy += text_a + ctx->padd;

		seek_row(self, GP_TABLE_ROW_ADVANCE, 1);
		cur_row++;

		gp_hline_xyw(ctx->buf, ix, cy - ctx->padd/2, iw, ctx->bg_color);
	}
}

static int move_down(gp_widget *self, const gp_widget_render_ctx *ctx,
                     unsigned int rows)
{
	gp_widget_table *tbl = self->tbl;

	if (!tbl->row_selected) {
		tbl->row_selected = 1;
		tbl->selected_row = tbl->start_row;

		fix_selected_row(tbl);

		goto redraw;
	}

	if (tbl->selected_row < tbl->last_rows) {
		tbl->selected_row += rows;

		fix_selected_row(tbl);

		goto redraw;
	}

	return 0;

redraw:
	rows = display_rows(self, ctx);

	if (tbl->selected_row >= tbl->start_row + rows)
		tbl->start_row = tbl->selected_row - rows + 1;

	gp_widget_send_widget_event(self, GP_WIDGET_TABLE_SELECT, (long)tbl->selected_row);

	gp_widget_redraw(self);
	return 1;
}

static int move_first(gp_widget *self)
{
	gp_widget_table *tbl = self->tbl;

	tbl->row_selected = 1;
	tbl->selected_row = 0;
	tbl->start_row = 0;

	gp_widget_send_widget_event(self, GP_WIDGET_TABLE_SELECT, (long)tbl->selected_row);
	gp_widget_redraw(self);

	return 1;
}

static int move_last(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	gp_widget_table *tbl = self->tbl;

	tbl->row_selected = 1;
	tbl->selected_row = last_row(self);
	tbl->start_row = tbl->selected_row - display_rows(self, ctx);

	gp_widget_send_widget_event(self, GP_WIDGET_TABLE_SELECT, (long)tbl->selected_row);
	gp_widget_redraw(self);

	return 1;
}

static int scroll_down(gp_widget *self, const gp_widget_render_ctx *ctx,
                       unsigned int rows)
{
	gp_widget_table *tbl = self->tbl;
	unsigned int disp_rows = display_rows(self, ctx);

	if (tbl->last_rows < disp_rows)
		return 0;

	unsigned int hidden_rows = tbl->last_rows - disp_rows;

	if (tbl->start_row >= hidden_rows)
		return 0;

	if (tbl->start_row + rows >= hidden_rows)
		tbl->start_row = hidden_rows;
	else
		tbl->start_row += rows;

	gp_widget_redraw(self);
	return 1;
}

static int scroll_up(gp_widget *self, unsigned int rows)
{
	gp_widget_table *tbl = self->tbl;

	if (!tbl->start_row)
		return 0;

	if (rows > tbl->start_row)
		tbl->start_row = 0;
	else
		tbl->start_row -= rows;

	gp_widget_redraw(self);
	return 1;
}

static int move_up(gp_widget *self, const gp_widget_render_ctx *ctx, unsigned int rows)
{
	gp_widget_table *tbl = self->tbl;

	if (!tbl->row_selected) {
		tbl->row_selected = 1;
		tbl->selected_row = tbl->start_row + display_rows(self, ctx) - 1;

		goto redraw;
	}

	if (tbl->selected_row > 0) {
		if (tbl->selected_row > rows)
			tbl->selected_row -= rows;
		else
			tbl->selected_row = 0;

		goto redraw;
	}

	return 0;

redraw:
	if (tbl->selected_row < tbl->start_row)
		tbl->start_row = tbl->selected_row;

	gp_widget_send_widget_event(self, GP_WIDGET_TABLE_SELECT, (long)tbl->selected_row);

	gp_widget_redraw(self);
	return 1;
}

static int header_click(gp_widget *self, const gp_widget_render_ctx *ctx, unsigned int x)
{
	gp_widget_table *tbl = self->tbl;
	unsigned int i, cx = 0;

	//TODO: inverval division?
	for (i = 0; i < tbl->cols-1; i++) {
		cx += tbl->cols_w[i].size + 2 * ctx->padd;

		if (x <= cx)
			break;
	}

	if (!col_is_sortable(tbl, i))
		return 0;

	if (tbl->sorted_by_col == i)
		tbl->sorted_desc = !tbl->sorted_desc;
	else
		tbl->sorted_by_col = i;

	sort_by_col(self, tbl->sorted_desc, i);
	gp_widget_redraw(self);

	return 1;
}

void gp_widget_table_sort_by(gp_widget *self, int desc, unsigned int col)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABLE, );

	gp_widget_table *tbl = self->tbl;

	int sort = 0;

	if (col >= tbl->cols) {
		GP_WARN("Column %u out of table, max = %u", col, tbl->cols);
		return;
	}

	if (!col_is_sortable(tbl, col)) {
		GP_WARN("Column %u not sortable", col);
		return;
	}

	if (tbl->sorted_by_col != col) {
		tbl->sorted_by_col = col;
		sort = 1;
	}

	if (tbl->sorted_desc != !!desc) {
		tbl->sorted_desc = !!desc;
		sort = 1;
	}

	if (sort) {
		sort_by_col(self, tbl->sorted_desc, col);
		gp_widget_redraw(self);
	}
}

static int row_click(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	gp_widget_table *tbl = self->tbl;
	unsigned int row = ev->st->cursor_y - header_h(self, ctx);

	row /= row_h(ctx);
	row += tbl->start_row;

	if (tbl->row_selected && tbl->selected_row == row) {
		if (gp_widgets_is_dclick(ev->time, tbl->last_click, ctx))
			gp_widget_send_widget_event(self, GP_WIDGET_TABLE_TRIGGER);
		goto ret;
	}

	tbl->selected_row = row;
	if (!tbl->row_selected)
		tbl->row_selected = 1;

	gp_widget_send_widget_event(self, GP_WIDGET_TABLE_SELECT, (long)tbl->selected_row);

	gp_widget_redraw(self);

ret:
	tbl->last_click = ev->time;
	return 1;
}

static int click(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	if (ev->st->cursor_y <= header_h(self, ctx))
		return header_click(self, ctx, ev->st->cursor_x);

	return row_click(self, ctx, ev);
}

static int enter(gp_widget *self)
{
	gp_widget_table *tbl = self->tbl;

	if (!tbl->row_selected)
		return 0;

	gp_widget_send_widget_event(self, GP_WIDGET_TABLE_TRIGGER);

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
			if (gp_ev_key_pressed(ev, GP_KEY_LEFT_SHIFT))
				return move_down(self, ctx, 10);

			return move_down(self, ctx, 1);
		break;
		case GP_KEY_UP:
			if (gp_ev_key_pressed(ev, GP_KEY_LEFT_SHIFT))
				return move_up(self, ctx, 10);

			return move_up(self, ctx, 1);
		break;
		}

		if (gp_widget_key_mod_pressed(ev))
			return 0;

		switch (ev->val) {
		case GP_KEY_PAGE_UP:
			return move_up(self, ctx, display_rows(self, ctx));
		case GP_KEY_PAGE_DOWN:
			return move_down(self, ctx, display_rows(self, ctx));
		case GP_KEY_END:
			return move_last(self, ctx);
		case GP_KEY_HOME:
			return move_first(self);
		case GP_BTN_LEFT:
			return click(self, ctx, ev);
		case GP_KEY_ENTER:
			return enter(self);
		}
	break;
	case GP_EV_REL:
		if (gp_widget_key_mod_pressed(ev))
			return 0;

		if (ev->code != GP_EV_REL_WHEEL)
			return 0;

		if (ev->val < 0)
			scroll_down(self, ctx, -ev->val);
		else
			scroll_up(self, ev->val);
	break;
	}

	return 0;
}

static void free_header(gp_widget_table_header *header, unsigned int cols)
{
	unsigned int i;

	for (i = 0; i < cols; i++)
		free(header[i].label);

	free(header);
}

enum header_keys {
	FILL,
	ID,
	LABEL,
	MIN_SIZE,
	ORDER,
	SORT,
	TATTR
};

static const gp_json_obj_attr header_attrs[] = {
	GP_JSON_OBJ_ATTR("fill", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("id", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("label", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("min_size", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("order", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("tattr", GP_JSON_STR),
};

static int parse_order(gp_json_reader *json, gp_json_val *val, int *desc)
{
	if (!strcmp(val->val_str, "asc")) {
		*desc = 0;
		return 1;
	}

	if (!strcmp(val->val_str, "desc")) {
		*desc = 1;
		return 1;
	}

	gp_json_warn(json, "Wrong order expected 'asc' or 'desc'");
	return 0;
}

static gp_widget_table_col_desc *table_col_map_get(gp_widget_table_col_desc *col_map, const char *id)
{
	if (!col_map)
		return NULL;

	for (;col_map->id; col_map++) {
		if (!strcmp(col_map->id, id))
			return col_map;
	}

	return NULL;
}

static const gp_json_obj header_obj_filter = {
	.attrs = header_attrs,
	.attr_cnt = GP_ARRAY_SIZE(header_attrs),
};

static gp_widget_table_header *parse_header(gp_json_reader *json, gp_json_val *val,
                                            gp_widget_table_col_desc *col_map,
                                            int *cols, int *sort_by_col, int *desc)
{
	gp_json_state state = gp_json_state_start(json);
	gp_widget_table_header *header;
	int cnt = 0;

	GP_JSON_ARR_FOREACH(json, val) {
		switch (val->type) {
		case GP_JSON_OBJ:
			cnt++;
			gp_json_obj_skip(json);
		break;
		default:
			gp_json_warn(json, "Invalid table header type!");
			//TODO skip array
		}
	}

	if (*cols != -1 && *cols != cnt) {
		gp_json_warn(json, "Table header size is not equal to number of columns");
		return NULL;
	}

	*cols = cnt;

	header = malloc(sizeof(*header) * (*cols));
	if (!header) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	memset(header, 0, sizeof(*header) * (*cols));

	gp_json_state_load(json, state);

	cnt = 0;

	GP_JSON_ARR_FOREACH(json, val) {
		//TODO SKIP ARR
		if (val->type != GP_JSON_OBJ)
			continue;

		header[cnt].tattr = GP_TATTR_BOLD;

		GP_JSON_OBJ_FILTER(json, val, &header_obj_filter, NULL) {
			switch (val->idx) {
			case FILL:
				header[cnt].col_fill = val->val_int;
			break;
			case ID:
				header[cnt].col_desc = table_col_map_get(col_map, val->val_str);

				if (col_map && !header[cnt].col_desc)
					gp_json_warn(json, "col_desc for id '%s' not found", val->val_str);
			break;
			case LABEL:
				header[cnt].label = strdup(val->val_str);
			break;
			case MIN_SIZE:
				header[cnt].col_min_size = val->val_int;
			break;
			case ORDER:
				if (*sort_by_col != -1) {
					gp_json_warn(json, "Default sort column already set");
				} else {
					if (parse_order(json, val, desc))
						*sort_by_col = cnt;
				}
			break;
			case TATTR:
				if (gp_widget_tattr_parse(val->val_str, &header[cnt].tattr, ~GP_TATTR_LARGE & (GP_TATTR_FONT | GP_TATTR_HALIGN)))
					gp_json_warn(json, "Invalid text attribute");
			break;
			}
		}

		cnt++;
	}

	return header;
}

enum keys {
	COL_OPS,
	COLS,
	HEADER,
	MIN_ROWS,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("col_ops", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("cols", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("header", GP_JSON_ARR),
	GP_JSON_OBJ_ATTR("min_rows", GP_JSON_INT),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_table(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	int cols = -1, min_rows = -1;
	gp_widget_table_header *table_header = NULL;
	int sort_by_col = -1, desc = 0;
	gp_widget_table_col_ops *col_ops = NULL;
	gp_widget_table_col_desc *col_map = NULL;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case COL_OPS:
			col_ops = gp_widget_struct_addr(val->val_str, ctx);
			if (col_ops)
				col_map = col_ops->col_map;
			else
				gp_json_warn(json, "No structure '%s' defined", val->val_str);
		break;
		case COLS:
			cols = val->val_int;
		break;
		case MIN_ROWS:
			min_rows = val->val_int;
		break;
		case HEADER:
			if (!col_map)
				gp_json_warn(json, "col_ops must be defined before header!");

			table_header = parse_header(json, val, col_map, &cols, &sort_by_col, &desc);
		break;
		}
	}

	if (!table_header)
		return NULL;

	if (cols < 0) {
		gp_json_warn(json, "Invalid or missing cols!");
		goto err;
	}

	if (min_rows < 0) {
		gp_json_warn(json, "Invalid or missing min_rows!");
		goto err;
	}

	if (!col_ops) {
		gp_json_warn(json, "No col_ops set!");
		goto err;
	}

	if (!col_ops->seek_row) {
		gp_json_warn(json, "col_ops->seek_row() callback not set!");
		goto err;
	}

	gp_widget *table = gp_widget_table_new(cols, min_rows, col_ops, table_header);
	if (!table)
		goto err;

	table->tbl->free = table_header;

	if (sort_by_col >= 0)
		gp_widget_table_sort_by(table, desc, sort_by_col);

	return table;
err:
	free_header(table_header, cols);
	return NULL;
}

static void free_(gp_widget *self)
{
	if (!self->tbl->free)
		return;

	free_header(self->tbl->free, self->tbl->cols);
}

struct gp_widget_ops gp_widget_table_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.distribute_w = distribute_w,
	.render = render,
	.event = event,
	.from_json = json_to_table,
	.free = free_,
	.id = "table",
};

static void set_header_flag(gp_widget *self)
{
	unsigned int i;

	self->tbl->needs_header = 0;

	for (i = 0; i < self->tbl->cols; i++) {
		if (self->tbl->header[i].label) {
			self->tbl->needs_header = 1;
			break;
		}

		if (col_is_sortable(self->tbl, i)) {
			self->tbl->needs_header = 1;
			break;
		}
	}
}

gp_widget *gp_widget_table_new(unsigned int cols, unsigned int min_rows,
			       const gp_widget_table_col_ops *col_ops,
                               const gp_widget_table_header *header)
{
	size_t size = sizeof(struct gp_widget_table);
	gp_widget *ret;

	size += cols * sizeof(gp_widget_table_col_size);

	ret = gp_widget_new(GP_WIDGET_TABLE, GP_WIDGET_CLASS_NONE, size);
	if (!ret)
		return NULL;

	ret->tbl->cols = cols;
	ret->tbl->min_rows = min_rows;
	ret->tbl->start_row = 0;
	ret->tbl->header = header;
	ret->tbl->cols_w = (void*)ret->tbl->buf;

	ret->tbl->col_ops.sort = col_ops->sort;
	ret->tbl->col_ops.get_cell = col_ops->get_cell;
	ret->tbl->col_ops.seek_row = col_ops->seek_row;

	if (col_ops->on_event)
		gp_widget_on_event_set(ret, col_ops->on_event, col_ops->on_event_priv);

	set_header_flag(ret);

	return ret;
}

void gp_widget_table_refresh(gp_widget *self)
{
	gp_widget_redraw(self);
}

void gp_widget_table_off_set(gp_widget *self, unsigned int off)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABLE, );

	self->tbl->start_row = off;

	gp_widget_redraw(self);
}

void gp_widget_table_sel_set(gp_widget *self, unsigned int row)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TABLE, );

	gp_widget_table *tbl = self->tbl;

	tbl->selected_row = row;
	if (!tbl->row_selected)
		tbl->row_selected = 1;

	gp_widget_send_widget_event(self, GP_WIDGET_TABLE_SELECT, (long)tbl->selected_row);

	gp_widget_redraw(self);
}
