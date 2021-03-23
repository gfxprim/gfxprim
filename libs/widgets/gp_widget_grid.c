//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <ctype.h>
#include <json-c/json.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_vec.h>
#include <utils/gp_matrix.h>
#include <gp_widgets.h>
#include <gp_widget_ops.h>
#include <gp_widget_json.h>

static gp_widget *widget_grid_grid_get(struct gp_widget_grid *grid,
                                       unsigned int col, unsigned int row)
{
	return grid->widgets[gp_matrix_idx(grid->rows, col, row)];
}

static gp_widget *widget_grid_get(gp_widget *self,
                                  unsigned int col, unsigned int row)
{
	return widget_grid_grid_get(self->grid, col, row);
}

static gp_widget *widget_grid_focused(gp_widget *self)
{
	return widget_grid_get(self,
	                       self->grid->focused_col,
	                       self->grid->focused_row);
}

static void widget_grid_focused_offset(gp_widget *self,
                                        gp_offset *offset)
{
	offset->x = self->grid->cols_off[self->grid->focused_col];
	offset->y = self->grid->rows_off[self->grid->focused_row];
}

static struct gp_widget *widget_grid_put(gp_widget *self, gp_widget *new,
		                         unsigned int x, unsigned int y)
{
	struct gp_widget_grid *g = self->grid;
	size_t idx = gp_matrix_idx(g->rows, x, y);

	gp_widget *ret = g->widgets[idx];

	g->widgets[idx] = new;

	gp_widget_set_parent(new, self);

	return ret;
}

static unsigned int padd_size(const gp_widget_render_ctx *ctx, int padd)
{
	return ctx->padd * padd;
}

static unsigned int min_w_uniform(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = self->grid;
	unsigned int x, sum_min_w = padd_size(ctx, grid->col_padds[0]);
	unsigned int max_cols_w = 0;

	for (x = 0; x < grid->cols; x++) {
		unsigned int y;
		for (y = 0; y < grid->rows; y++) {
			unsigned int min_w;
			min_w = gp_widget_min_w(widget_grid_get(self, x, y), ctx);
			max_cols_w = GP_MAX(max_cols_w, min_w);
		}

		sum_min_w += padd_size(ctx, grid->col_padds[x+1]);
	}

	return sum_min_w + grid->cols * max_cols_w;
}

static unsigned int min_w_(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = self->grid;
	unsigned int x, sum_min_w = padd_size(ctx, grid->col_padds[0]);

	for (x = 0; x < grid->cols; x++) {
		unsigned int y, max_col_w = 0;
		for (y = 0; y < grid->rows; y++) {
			unsigned int min_w;
			min_w = gp_widget_min_w(widget_grid_get(self, x, y), ctx);
			max_col_w = GP_MAX(max_col_w, min_w);
		}

		sum_min_w += max_col_w;
		sum_min_w += padd_size(ctx, grid->col_padds[x+1]);
	}

	return sum_min_w;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	if (self->grid->uniform)
		return min_w_uniform(self, ctx);

	return min_w_(self, ctx);
}

static unsigned int min_h_uniform(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = self->grid;
	unsigned int y, sum_min_h = padd_size(ctx, grid->row_padds[0]);
	unsigned int max_rows_h = 0;

	for (y = 0; y < grid->rows; y++) {
		unsigned int x;
		for (x = 0; x < grid->cols; x++) {
			unsigned int min_h;
			min_h = gp_widget_min_h(widget_grid_get(self, x, y), ctx);
			max_rows_h = GP_MAX(max_rows_h, min_h);
		}

		sum_min_h += padd_size(ctx, grid->row_padds[y+1]);
	}

	return sum_min_h + grid->rows * max_rows_h;
}

static unsigned int min_h_(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = self->grid;
	unsigned int y, sum_min_h = padd_size(ctx, grid->row_padds[0]);

	for (y = 0; y < grid->rows; y++) {
		unsigned int x, max_row_h = 0;
		for (x = 0; x < grid->cols; x++) {
			unsigned int min_h;
			min_h = gp_widget_min_h(widget_grid_get(self, x, y), ctx);
			max_row_h = GP_MAX(max_row_h, min_h);
		}

		sum_min_h += max_row_h;
		sum_min_h += padd_size(ctx, grid->row_padds[y+1]);
	}

	return sum_min_h;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	if (self->grid->uniform)
		return min_h_uniform(self, ctx);

	return min_h_(self, ctx);
}

static void compute_cols_rows_min_wh(struct gp_widget_grid *grid,
                                     const gp_widget_render_ctx *ctx)
{
	unsigned int x, y;

	for (y = 0; y < grid->rows; y++)
		grid->rows_h[y] = 0;

	for (x = 0; x < grid->cols; x++)
		grid->cols_w[x] = 0;

	for (y = 0; y < grid->rows; y++) {
		for (x = 0; x < grid->cols; x++) {
			struct gp_widget *widget = widget_grid_grid_get(grid, x, y);

			grid->cols_w[x] = GP_MAX(grid->cols_w[x], gp_widget_min_w(widget, ctx));
			grid->rows_h[y] = GP_MAX(grid->rows_h[y], gp_widget_min_h(widget, ctx));
		}
	}
}

static void compute_cols_rows_min_uniform_wh(struct gp_widget_grid *grid,
                                             const gp_widget_render_ctx *ctx)
{
	unsigned int x, y;
	unsigned int min_cols_w = 0, min_rows_h = 0;

	for (y = 0; y < grid->rows; y++) {
		for (x = 0; x < grid->cols; x++) {
			struct gp_widget *widget = widget_grid_grid_get(grid, x, y);

			min_cols_w = GP_MAX(min_cols_w, gp_widget_min_w(widget, ctx));
			min_rows_h = GP_MAX(min_rows_h, gp_widget_min_h(widget, ctx));
		}
	}

	for (x = 0; x < grid->cols; x++)
		grid->cols_w[x] = min_cols_w;

	for (y = 0; y < grid->rows; y++)
		grid->rows_h[y] = min_rows_h;
}

static void distribute_size(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	struct gp_widget_grid *grid = self->grid;
	unsigned int x, y;

	if (grid->uniform)
		compute_cols_rows_min_uniform_wh(grid, ctx);
	else
		compute_cols_rows_min_wh(grid, ctx);

	unsigned int sum_row_fills = 0;
	unsigned int sum_col_fills = 0;

	/* cell fills */
	for (y = 0; y < grid->rows; y++)
		sum_row_fills += grid->row_fills[y];

	for (x = 0; x < grid->cols; x++)
		sum_col_fills += grid->col_fills[x];

	/* padding fills */
	for (y = 0; y <= grid->rows; y++)
		sum_row_fills += grid->row_pfills[y];

	for (x = 0; x <= grid->cols; x++)
		sum_col_fills += grid->col_pfills[x];

	/* size to be distributed */
	unsigned int dx = self->w - self->min_w;
	unsigned int dy = self->h - self->min_h;

	if (sum_col_fills) {
		for (x = 0; x < grid->cols; x++)
			grid->cols_w[x] += dx * grid->col_fills[x] / sum_col_fills;
	}

	if (sum_row_fills) {
		for (y = 0; y < grid->rows; y++)
			grid->rows_h[y] += dy * grid->row_fills[y] / sum_row_fills;
	}

	/* Compute colum/row offsets */
	unsigned int cur_x = self->x + padd_size(ctx, grid->col_padds[0]);
	if (sum_col_fills)
		cur_x += dx * grid->col_pfills[0] / sum_col_fills;

	for (x = 0; x < grid->cols; x++) {
		grid->cols_off[x] = cur_x;
		cur_x += grid->cols_w[x] + padd_size(ctx, grid->col_padds[x+1]);
		if (sum_col_fills)
			cur_x += dx * grid->col_pfills[x+1] / sum_col_fills;
	}

	unsigned int cur_y = self->y + padd_size(ctx, grid->row_padds[0]);
	if (sum_row_fills)
		cur_y += dy * grid->row_pfills[0] / sum_row_fills;

	for (y = 0; y < grid->rows; y++) {
		grid->rows_off[y] = cur_y;
		cur_y += grid->rows_h[y] + padd_size(ctx, grid->row_padds[y+1]);
		if (sum_row_fills)
			cur_y += dy * grid->row_pfills[y+1] / sum_row_fills;
	}

	/* Place the widgets */
	for (y = 0; y < grid->rows; y++) {
		for (x = 0; x < grid->cols; x++) {
			struct gp_widget *widget = widget_grid_get(self, x, y);

			if (widget) {
				gp_widget_ops_distribute_size(widget, ctx,
				                              grid->cols_w[x],
				                              grid->rows_h[y],
				                              new_wh);
			}
		}
	}
}

static void fill_padding(gp_widget *self, const gp_offset *offset,
                         const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = self->grid;

	GP_DEBUG(3, "Filling grid %p padding", self);

	gp_coord x_off = self->x + offset->x;
	gp_coord y_off = self->y + offset->y;
	gp_coord end_y = y_off + self->h - 1;
	gp_coord end_x = x_off + self->w - 1;

	unsigned int y, cur_y = y_off;
	for (y = 0; y < grid->rows; y++) {
		gp_fill_rect_xyxy(ctx->buf, x_off, cur_y,
		                  end_x, offset->y + grid->rows_off[y] - 1,
				  ctx->bg_color);

		if (y < grid->rows)
			cur_y = grid->rows_h[y] + grid->rows_off[y] + offset->y;
	}

	gp_fill_rect_xyxy(ctx->buf, x_off, cur_y,
	                  end_x, end_y, ctx->bg_color);

	unsigned int x, cur_x = x_off;
	for (x = 0; x < grid->cols; x++) {
		if (grid->cols_off[x]) {
			gp_fill_rect_xyxy(ctx->buf, cur_x, y_off,
			                  offset->x + grid->cols_off[x] - 1, end_y,
					  ctx->bg_color);
		}

		if (x < grid->cols)
			cur_x = grid->cols_off[x] + grid->cols_w[x] + offset->x;
	}

	gp_fill_rect_xyxy(ctx->buf, cur_x, y_off,
		          end_x, end_y, ctx->bg_color);

	if (grid->frame) {
		gp_rrect_xywh(ctx->buf, x_off, y_off,
			      self->w, self->h, ctx->text_color);
	}
}

/* Fill unused space between grid and widget */
static void fill_unused(gp_widget *widget, const gp_widget_render_ctx *ctx,
                        unsigned int cur_x, unsigned int cur_y,
			unsigned int cur_w, unsigned int cur_h)
{
	gp_pixel bg = ctx->bg_color;

	GP_DEBUG(4, "Filling unused space around widget %p", widget);

	if (widget->x) {
		gp_fill_rect_xywh(ctx->buf, cur_x, cur_y,
				  widget->x, cur_h, bg);
	}

	if (widget->y) {
		gp_fill_rect_xywh(ctx->buf, cur_x + widget->x, cur_y,
				  widget->w, widget->y, bg);
	}

	unsigned int wid_end_x = widget->x + widget->w;
	unsigned int wid_after_w = cur_w - wid_end_x;

	if (wid_after_w) {
		gp_fill_rect_xywh(ctx->buf, cur_x + wid_end_x, cur_y,
				  wid_after_w, cur_h, bg);
	}

	unsigned int wid_end_y = widget->y + widget->h;
	unsigned int wid_after_h = cur_h - wid_end_y;

	if (wid_after_h) {
		gp_fill_rect_xywh(ctx->buf, cur_x + widget->x, cur_y + wid_end_y,
				  widget->w, wid_after_h, bg);
	}
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct gp_widget_grid *grid = self->grid;
	gp_coord cur_x, cur_y;
	unsigned int x, y;

	if (gp_widget_should_redraw(self, flags)) {
		fill_padding(self, offset, ctx);
		gp_widget_ops_blit(ctx,
		                   self->x + offset->x, self->y + offset->y,
		                   self->w, self->h);
	}

	for (y = 0; y < grid->rows; y++) {
		cur_y = grid->rows_off[y] + offset->y;

		for (x = 0; x < grid->cols; x++) {
			cur_x = grid->cols_off[x] + offset->x;

			struct gp_widget *widget = widget_grid_get(self, x, y);

			if (!widget) {
				if (gp_widget_should_redraw(self, flags)) {
					gp_fill_rect_xywh(ctx->buf, cur_x, cur_y,
							  grid->cols_w[x], grid->rows_h[y],
							  ctx->bg_color);
				}
				continue;
			}

			if (gp_widget_should_redraw(self, flags)) {
				fill_unused(widget, ctx, cur_x, cur_y,
				            grid->cols_w[x], grid->rows_h[y]);
			}

			if (!widget->redraw_child &&
			    !gp_widget_should_redraw(widget, flags))
				continue;

			GP_DEBUG(3, "Rendering widget %s [%u:%u]",
			         gp_widget_type_id(widget), x, y);

			gp_offset child_offset = {
				.x = cur_x,
				.y = cur_y,
			};

			gp_widget_ops_render(widget, &child_offset, ctx, flags);
		}
	}
/*
	gp_pixel col = random();

	unsigned int sx = grid->cols_off[0] + offset->x;
	unsigned int ex = grid->cols_off[grid->cols-1] + grid->cols_w[grid->cols-1] + offset->x;

	for (y = 0; y < grid->rows; y++) {
		gp_hline_xxy(ctx->buf, sx, ex, grid->rows_off[y] + offset->y, col);
		gp_hline_xxy(ctx->buf, sx, ex, grid->rows_off[y] + grid->rows_h[y] + offset->y, col);
	}

	unsigned int sy = grid->rows_off[0] + offset->y;
	unsigned int ey = grid->rows_off[grid->rows-1] + grid->rows_h[grid->rows-1] + offset->y;

	for (x = 0; x < grid->cols; x++) {
		gp_vline_xyy(ctx->buf, grid->cols_off[x] + offset->x, sy, ey, col);
		gp_vline_xyy(ctx->buf, grid->cols_off[x] + grid->cols_w[x] + offset->x, sy, ey, col);
	}
*/
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	struct gp_widget *w = widget_grid_focused(self);

	GP_DEBUG(3, "event widget %p (%s)", w, gp_widget_type_id(w));

	gp_offset offset;

	widget_grid_focused_offset(self, &offset);

	offset.x -= self->x;
	offset.y -= self->y;

	return gp_widget_ops_event_offset(w, ctx, ev, offset.x, offset.y);
}

static int try_focus(gp_widget *self, unsigned int col, unsigned int row, int sel)
{
	gp_widget *w = widget_grid_get(self, col, row);

	GP_DEBUG(4, "Trying to focus widget %p (%s) %ux%u",
		 w, gp_widget_type_id(w), col, row);

	if (!gp_widget_ops_render_focus(widget_grid_get(self, col, row), sel))
		return 0;

	gp_widget_ops_render_focus(widget_grid_focused(self), GP_FOCUS_OUT);

	self->grid->focused_col = col;
	self->grid->focused_row = row;

	return 1;
}

static int focus_left(gp_widget *self, int sel)
{
	unsigned int col = self->grid->focused_col;
	unsigned int row = self->grid->focused_row;

	for (;;) {
		if (col == 0)
			return 0;

		if (try_focus(self, --col, row, sel))
			return 1;
	}
}

static int focus_right(gp_widget *self, int sel)
{
	unsigned int col = self->grid->focused_col;
	unsigned int row = self->grid->focused_row;

	for (;;) {
		if (++col >= self->grid->cols)
			return 0;

		if (try_focus(self, col, row, sel))
			return 1;
	}
}

static int focus_up(gp_widget *self, int sel)
{
	unsigned int col = self->grid->focused_col;
	unsigned int row = self->grid->focused_row;

	for (;;) {
		if (row == 0)
			return 0;

		if (try_focus(self, col, --row, sel))
			return 1;
	}
}

static int focus_down(gp_widget *self, int sel)
{
	unsigned int col = self->grid->focused_col;
	unsigned int row = self->grid->focused_row;

	for (;;) {
		if (++row >= self->grid->rows)
			return 0;

		if (try_focus(self, col, row, sel))
			return 1;
	}
}

static int focus_prev(gp_widget *self, int sel)
{
	unsigned int col = self->grid->focused_col;
	unsigned int row = self->grid->focused_row;

	for (;;) {
		if (col > 0) {
			col--;
		} else if (row > 0) {
			row--;
			col = self->grid->cols - 1;
		} else {
			return 0;
		}

		if (try_focus(self, col, row, sel))
			return 1;
	}
}

static int focus_next(gp_widget *self, int sel)
{
	unsigned int col = self->grid->focused_col;
	unsigned int row = self->grid->focused_row;

	for (;;) {
		if (col + 1 < self->grid->cols) {
			col++;
		} else if (row + 1 < self->grid->rows) {
			col = 0;
			row++;
		} else {
			return 0;
		}

		if (try_focus(self, col, row, sel))
			return 1;
	}
}

static int focus_in(gp_widget *self, int sel)
{
	if (!self->grid->focused)
		return self->grid->focused = focus_next(self, sel);

	return try_focus(self, self->grid->focused_col, self->grid->focused_row, sel);
}

static int focus(gp_widget *self, int sel)
{
	gp_widget *w = widget_grid_focused(self);

	if (gp_widget_ops_render_focus(w, sel))
		return 1;

	switch (sel) {
	case GP_FOCUS_IN:
		return focus_in(self, sel);
	case GP_FOCUS_NEXT:
		return focus_next(self, sel);
	case GP_FOCUS_PREV:
		return focus_prev(self, sel);
	case GP_FOCUS_UP:
		return focus_up(self, sel);
	case GP_FOCUS_DOWN:
		return focus_down(self, sel);
	case GP_FOCUS_LEFT:
		return focus_left(self, sel);
	case GP_FOCUS_RIGHT:
		return focus_right(self, sel);
	}

	return 0;
}

static int coord_search(unsigned int coord,
                        unsigned int *sizes, unsigned int *offsets,
                        unsigned int len)
{
	unsigned int i;

	if (coord < offsets[0])
		return -1;

	//TODO interval divison?
	for (i = 0; i < len; i++) {
		if (coord >= offsets[i] && coord <= offsets[i] + sizes[i])
			return i;
	}

	return -1;
}

static int focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                    unsigned int x, unsigned int y)
{
	int col, row;
	struct gp_widget_grid *grid = self->grid;

	col = coord_search(x + self->x, grid->cols_w, grid->cols_off, grid->cols);
	row = coord_search(y + self->y, grid->rows_h, grid->rows_off, grid->rows);

	if (col < 0 || row < 0)
		return 0;

	x -= self->grid->cols_off[col] - self->x;
	y -= self->grid->rows_off[row] - self->y;

	if (!gp_widget_ops_render_focus_xy(widget_grid_get(self, col, row), ctx, x, y))
		return 0;

	if (grid->focused_col != (unsigned int)col || grid->focused_row != (unsigned int)row)
		gp_widget_ops_render_focus(widget_grid_focused(self), GP_FOCUS_OUT);

	grid->focused_col = col;
	grid->focused_row = row;

	return 1;
}

static void set_hborder_padd(gp_widget *self, uint8_t border)
{
	self->grid->col_padds[0] = border;
	self->grid->col_padds[self->grid->cols] = border;
}

static void set_vborder_padd(gp_widget *self, uint8_t border)
{
	self->grid->row_padds[0] = border;
	self->grid->row_padds[self->grid->rows] = border;
}

static void set_border_padd(gp_widget *self, uint8_t border)
{
	set_vborder_padd(self, border);
	set_hborder_padd(self, border);
}

static void set_hborder_fill(gp_widget *self, uint8_t border)
{
	self->grid->col_fills[0] = border;
	self->grid->col_fills[self->grid->cols] = border;
}

static void set_vborder_fill(gp_widget *self, uint8_t border)
{
	self->grid->row_fills[0] = border;
	self->grid->row_fills[self->grid->rows] = border;
}

static void set_border_fill(gp_widget *self, uint8_t border)
{
	set_vborder_fill(self, border);
	set_hborder_fill(self, border);
}

static void set_rpad(gp_widget *self, uint8_t pad)
{
	unsigned int i;

	for (i = 1; i < self->grid->rows; i++)
		self->grid->row_padds[i] = pad;
}

static void set_cpad(gp_widget *self, uint8_t pad)
{
	unsigned int i;

	for (i = 1; i < self->grid->cols; i++)
		self->grid->col_padds[i] = pad;
}

static void set_pad(gp_widget *self, uint8_t pad)
{
	set_rpad(self, pad);
	set_cpad(self, pad);
}

static int get_uint8(const char *str, uint8_t *val, char **end,
                     const char *sarray, const char *name)
{
	long v = strtol(str, end, 10);

	if (str == *end) {
		GP_WARN("%s: Expected number '%s':%li",
			name, sarray, (long)(str-sarray));
		return 1;
	}

	if (v < 0 || v > 0xff) {
		GP_WARN("%s: Number outside of bounds '%li' at '%s':%li",
			name, v, sarray, (long)(str - sarray));
		return 1;
	}

	*val = v;

	return 0;
}

static void parse_strarray(const char *sarray, uint8_t *array, unsigned int len,
                           const char *name)
{
	const char *str = sarray;
	unsigned int i = 0;

	if (!sarray)
		return;

	for (;;) {
		char *end;
		uint8_t val;

		if (get_uint8(str, &val, &end, sarray, name))
			return;

		str = end;

		while (isspace(*str))
			str++;

		switch (*str) {
		case ',':
			if (i >= len) {
				GP_WARN("%s: Index out of bounds at '%s':%li",
				        name, sarray, (long)(str - sarray));
				return;
			}
			array[i++] = val;
			str++;
			continue;
		break;
		case '*':
		break;
		case '\0':
			if (i >= len) {
				GP_WARN("%s: Index out of bounds at '%s':%li",
				        name, sarray, (long)(str - sarray));
				return;
			}

			array[i++] = val;

			if (i != len) {
				GP_WARN("%s: Array too short expected %u numbers.",
					name, len);
			}

			return;
		default:
			GP_WARN("%s: Unexpected character '%c' at '%s':%li",
			        name, *str, sarray, (long)(str - sarray));
			return;
		}

		unsigned int j, mul = val;

		str++;

		if (get_uint8(str, &val, &end, sarray, name))
			return;

		for (j = 0; j < mul; j++) {
			if (i >= len) {
				GP_WARN("%s: Index out of bounds at '%s':%li",
			                name, sarray, (long)(str - sarray));
				return;
			}
			array[i++] = val;
		}

		str = end;

		while (isspace(*str))
			str++;

		if (*str != ',' && *str != '\0') {
			GP_WARN("%s: Unexpected character '%c' at '%s':%li",
			        name, *str, sarray, (long)(str - sarray));
			return;
		}

		if (*str == '\0') {
			if (i != len) {
				GP_WARN("%s: Array too short expected %u numbers.",
					name, len);
			}
			return;
		}

		str++;
	}
}

static gp_widget *json_to_grid(json_object *json, void **uids)
{
	int cols = 0, rows = 0, frame = 0, pad = -1;
	json_object *widgets = NULL;
	const char *border = NULL;
	const char *cpad = NULL;
	const char *rpad = NULL;
	const char *cpadf = NULL;
	const char *rpadf = NULL;
	const char *cfill = NULL;
	const char *rfill = NULL;
	int uniform = 0;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "cols"))
			cols = json_object_get_int(val);
		else if (!strcmp(key, "rows"))
			rows = json_object_get_int(val);
		else if (!strcmp(key, "widgets"))
			widgets = val;
		else if (!strcmp(key, "border"))
			border = json_object_get_string(val);
		else if (!strcmp(key, "pad"))
			pad = json_object_get_int(val);
		else if (!strcmp(key, "cpad"))
			cpad = json_object_get_string(val);
		else if (!strcmp(key, "rpad"))
			rpad = json_object_get_string(val);
		else if (!strcmp(key, "cpadf"))
			cpadf = json_object_get_string(val);
		else if (!strcmp(key, "rpadf"))
			rpadf = json_object_get_string(val);
		else if (!strcmp(key, "cfill"))
			cfill = json_object_get_string(val);
		else if (!strcmp(key, "rfill"))
			rfill = json_object_get_string(val);
		else if (!strcmp(key, "frame"))
			frame = !!json_object_get_int(val);
		else if (!strcmp(key, "uniform"))
			uniform = 1;
		else
			GP_WARN("Invalid grid key '%s'", key);
	}

	if (!cols)
		cols = 1;

	if (!rows)
		rows = 1;

	if (cols <= 0 || rows <= 0 || !widgets) {
		GP_WARN("Invalid grid widget!");
		return NULL;
	}

	gp_widget *grid = gp_widget_grid_new(cols, rows);
	if (!grid)
		return NULL;

	grid->grid->frame = frame;
	grid->grid->uniform = uniform;

	if (pad >= 0)
		set_pad(grid, pad);

	parse_strarray(cpad, grid->grid->col_padds, cols+1, "Grid cpad");
	parse_strarray(rpad, grid->grid->row_padds, rows+1, "Grid rpad");
	parse_strarray(cpadf, grid->grid->col_pfills, cols+1, "Grid cpadf");
	parse_strarray(rpadf, grid->grid->row_pfills, rows+1, "Grid rpadf");
	parse_strarray(cfill, grid->grid->col_fills, cols, "Grid cfill");
	parse_strarray(rfill, grid->grid->row_fills, rows, "Grid rfill");

	if (border) {
		if (!strcmp(border, "horiz")) {
			set_vborder_padd(grid, 0);
		} else if (!strcmp(border, "vert")) {
			set_hborder_padd(grid, 0);
		} else if (!strcmp(border, "none")) {
			set_border_padd(grid, 0);
		} else if (!strcmp(border, "all")) {
			//default
		} else {
			int b = atoi(border);

			if (b > 0)
				set_border_padd(grid, b);
			else
				GP_WARN("Invalid border '%s'", border);
		}
	}

	if (!json_object_is_type(widgets, json_type_array)) {
		GP_WARN("Grid key widgets has to be array!");
		return grid;
	}

	int col, row;

	for (col = 0; col < cols; col++) {
		for (row = 0; row < rows; row++) {
			json_object *json_widget = json_object_array_get_idx(widgets, col * rows + row);

			if (!json_widget) {
				GP_WARN("Not enough widgets to fill grid!");
				return grid;
			}

			gp_widget *widget = gp_widget_from_json(json_widget, uids);

			if (widget)
				gp_widget_grid_put(grid, col, row, widget);
		}
	}

	if (json_object_array_get_idx(widgets, cols * rows))
		GP_WARN("Too many widgets in grid!");

	return grid;
}

static void free_(gp_widget *self)
{
	gp_matrix_free(self->grid->widgets);

	gp_vec_free(self->grid->cols_w);
	gp_vec_free(self->grid->rows_h);
	gp_vec_free(self->grid->cols_off);
	gp_vec_free(self->grid->rows_off);
	gp_vec_free(self->grid->col_padds);
	gp_vec_free(self->grid->row_padds);
	gp_vec_free(self->grid->col_pfills);
	gp_vec_free(self->grid->row_pfills);
	gp_vec_free(self->grid->col_fills);
	gp_vec_free(self->grid->row_fills);
}

static void for_each_child(gp_widget *self, void (*func)(gp_widget *child))
{
	unsigned int x, y;
	struct gp_widget_grid *grid = self->grid;

	for (y = 0; y < grid->rows; y++) {
		for (x = 0; x < grid->cols; x++) {
			gp_widget *child = widget_grid_get(self, x, y);

			if (child)
				func(child);
		}
	}
}

struct gp_widget_ops gp_widget_grid_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.free = free_,
	.focus = focus,
	.focus_xy = focus_xy,
	.distribute_size = distribute_size,
	.for_each_child = for_each_child,
	.from_json = json_to_grid,
	.id = "grid",
};

gp_widget *gp_widget_grid_new(unsigned int cols, unsigned int rows)
{
	unsigned int i;
	gp_widget *ret;

	ret = gp_widget_new(GP_WIDGET_GRID, GP_WIDGET_CLASS_NONE, sizeof(struct gp_widget_grid));
	if (!ret)
		return NULL;

	ret->grid->cols = cols;
	ret->grid->rows = rows;
	ret->grid->widgets = gp_matrix_new(cols, rows, sizeof(gp_widget*));

	ret->grid->cols_w = gp_vec_new(cols, sizeof(unsigned int));
	ret->grid->rows_h = gp_vec_new(rows, sizeof(unsigned int));
	ret->grid->cols_off = gp_vec_new(cols, sizeof(unsigned int));
	ret->grid->rows_off = gp_vec_new(rows, sizeof(unsigned int));
	ret->grid->col_padds = gp_vec_new(cols + 1, sizeof(uint8_t));
	ret->grid->row_padds = gp_vec_new(rows + 1, sizeof(uint8_t));
	ret->grid->col_pfills = gp_vec_new(cols + 1, sizeof(uint8_t));
	ret->grid->row_pfills = gp_vec_new(rows + 1, sizeof(uint8_t));
	ret->grid->col_fills = gp_vec_new(cols, sizeof(uint8_t));
	ret->grid->row_fills = gp_vec_new(rows, sizeof(uint8_t));

	for (i = 0; i <= cols; i++)
		ret->grid->col_padds[i] = 1;

	for (i = 0; i < cols; i++)
		ret->grid->col_fills[i] = 1;

	for (i = 0; i <= rows; i++)
		ret->grid->row_padds[i] = 1;

	for (i = 0; i < rows; i++)
		ret->grid->row_fills[i] = 1;

	return ret;
}

static int assert_col_row(gp_widget *self, unsigned int col, unsigned int row)
{
	if (col >= self->grid->cols) {
		GP_BUG("Invalid column index %u Grid %p %ux%u",
			col, self, self->grid->cols, self->grid->rows);
		return 1;
	}

	if (row >= self->grid->rows) {
		GP_BUG("Invalid row index %u Grid %p %ux%u",
			row, self, self->grid->cols, self->grid->rows);
		return 1;
	}

	return 0;
}

gp_widget *gp_widget_grid_put(gp_widget *self, unsigned int col, unsigned int row,
                              gp_widget *widget)
{
	gp_widget *ret;

	GP_WIDGET_ASSERT(self, GP_WIDGET_GRID, NULL);

	if (assert_col_row(self, col, row))
		return NULL;

	ret = widget_grid_put(self, widget, col, row);
	if (ret)
		ret->parent = NULL;

	gp_widget_resize(self);
	gp_widget_redraw(widget);

	return ret;
}

void gp_widget_grid_insert_rows(gp_widget *self, unsigned int row, unsigned int rows)
{
	size_t i;

	GP_WIDGET_ASSERT(self, GP_WIDGET_GRID, );

	struct gp_widget_grid *g = self->grid;

	if (row > g->rows) {
		GP_WARN("Invalid grid (%p) row %u (rows %u)", self, row, g->rows);
		return;
	}

	g->widgets = gp_matrix_insert_rows(g->widgets, g->cols, g->rows, row, rows);

	g->rows_h = gp_vec_insert(g->rows_h, row, rows);
	g->rows_off = gp_vec_insert(g->rows_off, row, rows);
	g->row_padds = gp_vec_insert(g->row_padds, row, rows);
	g->row_pfills = gp_vec_insert(g->row_pfills, row, rows);
	g->row_fills = gp_vec_insert(g->row_fills, row, rows);

	for (i = row; i < row + rows; i++)
		g->row_padds[i] = 1;

	for (i = row; i < row + rows; i++)
		g->row_fills[i] = 1;

	g->rows += rows;

	gp_widget_resize(self);
}


void gp_widget_grid_add_rows(gp_widget *self, unsigned int rows)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_GRID, );

	gp_widget_grid_insert_rows(self, self->grid->rows, rows);
}

gp_widget *gp_widget_grid_rem(gp_widget *self, unsigned int col, unsigned int row)
{
	gp_widget *ret;

	GP_WIDGET_ASSERT(self, GP_WIDGET_GRID, NULL);

	if (assert_col_row(self, col, row))
		return NULL;

	ret = widget_grid_put(self, NULL, col, row);
	if (ret)
		ret->parent = NULL;

	if (self->grid->focused_col == col && self->grid->focused_row == row) {
		self->grid->focused_col = 0;
		self->grid->focused_row = 0;
		self->grid->focused = 0;
	}

	gp_widget_resize(self);
	gp_widget_redraw(self);

	return ret;
}

gp_widget *gp_widget_grid_get(gp_widget *self, unsigned int col, unsigned int row)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_GRID, NULL);

	if (assert_col_row(self, col, row))
		return NULL;

	return widget_grid_get(self, col, row);
}

void gp_widget_grid_border_set(gp_widget *self, unsigned int padd, unsigned int fill)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_GRID, );

	set_border_padd(self, padd);
	set_border_fill(self, fill);
}

void gp_widget_grid_hborder_set(gp_widget *self, unsigned int padd, unsigned int fill)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_GRID, );

	set_hborder_padd(self, padd);
	set_hborder_fill(self, fill);
}

void gp_widget_grid_vborder_set(gp_widget *self, unsigned int padd, unsigned int fill)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_GRID, );

	set_vborder_padd(self, padd);
	set_vborder_fill(self, fill);
}
