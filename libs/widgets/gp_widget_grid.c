//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <ctype.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_vec.h>
#include <utils/gp_matrix.h>
#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_json.h>

#include "gp_widget_grid_priv.h"

static gp_widget *widget_grid_grid_get(struct gp_widget_grid *grid,
                                       unsigned int col, unsigned int row)
{
	return grid->widgets[gp_matrix_idx(grid->rows, col, row)];
}

static gp_widget *widget_grid_get(gp_widget *self,
                                  unsigned int col, unsigned int row)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	return widget_grid_grid_get(grid, col, row);
}

static gp_widget *widget_grid_focused(gp_widget *self)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (!grid->cols || !grid->rows)
		return NULL;

	return widget_grid_get(self, grid->focused_col, grid->focused_row);
}

static void widget_grid_focused_offset(gp_widget *self, gp_offset *offset)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	offset->x = grid->col_s[grid->focused_col].off;
	offset->y = grid->row_s[grid->focused_row].off;
}

static int widget_grid_child_lookup(gp_widget *self, gp_widget *child,
                                    unsigned int *col, unsigned int *row)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	for (*col = 0; *col < grid->cols; (*col)++) {
		for (*row = 0; *row < grid->rows; (*row)++) {
			if (child == widget_grid_get(self, *col, *row))
				return 1;
		}
	}

	return 0;

}

static struct gp_widget *widget_grid_put(gp_widget *self, gp_widget *new,
		                         unsigned int x, unsigned int y)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	size_t idx = gp_matrix_idx(grid->rows, x, y);

	gp_widget *ret = grid->widgets[idx];

	grid->widgets[idx] = new;

	gp_widget_set_parent(new, self);

	return ret;
}

static unsigned int padd_size(const gp_widget_render_ctx *ctx, int padd)
{
	return ctx->padd * padd;
}

static unsigned int min_w_uniform(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	unsigned int x, sum_min_w = padd_size(ctx, grid->col_b[0].padd);
	unsigned int max_cols_w = 0;

	for (x = 0; x < grid->cols; x++) {
		unsigned int y;
		for (y = 0; y < grid->rows; y++) {
			unsigned int min_w;
			min_w = gp_widget_min_w(widget_grid_get(self, x, y), ctx);
			max_cols_w = GP_MAX(max_cols_w, min_w);
		}

		sum_min_w += padd_size(ctx, grid->col_b[x+1].padd);
	}

	return sum_min_w + grid->cols * max_cols_w;
}

static unsigned int min_w_(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	unsigned int x, sum_min_w = padd_size(ctx, grid->col_b[0].padd);

	for (x = 0; x < grid->cols; x++) {
		unsigned int y, max_col_w = 0;
		for (y = 0; y < grid->rows; y++) {
			unsigned int min_w;
			min_w = gp_widget_min_w(widget_grid_get(self, x, y), ctx);
			max_col_w = GP_MAX(max_col_w, min_w);
		}

		sum_min_w += max_col_w;
		sum_min_w += padd_size(ctx, grid->col_b[x+1].padd);
	}

	return sum_min_w;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (grid->flags & GP_WIDGET_GRID_UNIFORM)
		return min_w_uniform(self, ctx);

	return min_w_(self, ctx);
}

static unsigned int min_h_uniform(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	unsigned int y, sum_min_h = padd_size(ctx, grid->row_b[0].padd);
	unsigned int max_rows_h = 0;

	for (y = 0; y < grid->rows; y++) {
		unsigned int x;
		for (x = 0; x < grid->cols; x++) {
			unsigned int min_h;
			min_h = gp_widget_min_h(widget_grid_get(self, x, y), ctx);
			max_rows_h = GP_MAX(max_rows_h, min_h);
		}

		sum_min_h += padd_size(ctx, grid->row_b[y+1].padd);
	}

	return sum_min_h + grid->rows * max_rows_h;
}

static unsigned int min_h_(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	unsigned int y, sum_min_h = padd_size(ctx, grid->row_b[0].padd);

	for (y = 0; y < grid->rows; y++) {
		unsigned int x, max_row_h = 0;
		for (x = 0; x < grid->cols; x++) {
			unsigned int min_h;
			min_h = gp_widget_min_h(widget_grid_get(self, x, y), ctx);
			max_row_h = GP_MAX(max_row_h, min_h);
		}

		sum_min_h += max_row_h;
		sum_min_h += padd_size(ctx, grid->row_b[y+1].padd);
	}

	return sum_min_h;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (grid->flags & GP_WIDGET_GRID_UNIFORM)
		return min_h_uniform(self, ctx);

	return min_h_(self, ctx);
}

static void compute_cols_min_w(struct gp_widget_grid *grid,
                               const gp_widget_render_ctx *ctx)
{
	unsigned int x, y;

	for (x = 0; x < grid->cols; x++)
		grid->col_s[x].size = 0;

	for (y = 0; y < grid->rows; y++) {
		for (x = 0; x < grid->cols; x++) {
			gp_widget *widget = widget_grid_grid_get(grid, x, y);

			grid->col_s[x].size = GP_MAX(grid->col_s[x].size, gp_widget_min_w(widget, ctx));
		}
	}
}

static void compute_rows_min_h(struct gp_widget_grid *grid,
                               const gp_widget_render_ctx *ctx)
{
	unsigned int x, y;

	for (y = 0; y < grid->rows; y++)
		grid->row_s[y].size = 0;

	for (y = 0; y < grid->rows; y++) {
		for (x = 0; x < grid->cols; x++) {
			gp_widget *widget = widget_grid_grid_get(grid, x, y);

			grid->row_s[y].size = GP_MAX(grid->row_s[y].size, gp_widget_min_h(widget, ctx));
		}
	}
}

static void compute_cols_min_uniform_w(struct gp_widget_grid *grid,
                                       const gp_widget_render_ctx *ctx)
{
	unsigned int min_cols_w = 0;
	unsigned int x, y;

	for (y = 0; y < grid->rows; y++) {
		for (x = 0; x < grid->cols; x++) {
			struct gp_widget *widget = widget_grid_grid_get(grid, x, y);

			min_cols_w = GP_MAX(min_cols_w, gp_widget_min_w(widget, ctx));
		}
	}

	for (x = 0; x < grid->cols; x++)
		grid->col_s[x].size = min_cols_w;
}

static void compute_rows_min_uniform_h(struct gp_widget_grid *grid,
                                       const gp_widget_render_ctx *ctx)
{
	unsigned int min_rows_h = 0;
	unsigned int x, y;

	for (y = 0; y < grid->rows; y++) {
		for (x = 0; x < grid->cols; x++) {
			struct gp_widget *widget = widget_grid_grid_get(grid, x, y);

			min_rows_h = GP_MAX(min_rows_h, gp_widget_min_h(widget, ctx));
		}
	}

	for (y = 0; y < grid->rows; y++)
		grid->row_s[y].size = min_rows_h;
}

static void distribute_w(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	unsigned int x, y;

	if (grid->flags & GP_WIDGET_GRID_UNIFORM)
		compute_cols_min_uniform_w(grid, ctx);
	else
		compute_cols_min_w(grid, ctx);

	unsigned int sum_col_fills = 0;

	/* cell fills */
	for (x = 0; x < grid->cols; x++)
		sum_col_fills += grid->col_s[x].fill;

	/* padding fills */
	for (x = 0; x <= grid->cols; x++)
		sum_col_fills += grid->col_b[x].fill;

	/* size to be distributed */
	unsigned int dx = self->w - self->min_w;

	if (sum_col_fills) {
		for (x = 0; x < grid->cols; x++)
			grid->col_s[x].size += dx * grid->col_s[x].fill / sum_col_fills;
	}

	/* Compute colum offsets */
	unsigned int cur_x = self->x + padd_size(ctx, grid->col_b[0].padd);
	if (sum_col_fills)
		cur_x += dx * grid->col_b[0].fill / sum_col_fills;

	for (x = 0; x < grid->cols; x++) {
		grid->col_s[x].off = cur_x;
		cur_x += grid->col_s[x].size + padd_size(ctx, grid->col_b[x+1].padd);
		if (sum_col_fills)
			cur_x += dx * grid->col_b[x+1].fill / sum_col_fills;
	}

	/* Place the widgets */
	for (y = 0; y < grid->rows; y++) {
		for (x = 0; x < grid->cols; x++) {
			gp_widget *widget = widget_grid_get(self, x, y);

			if (widget) {
				gp_widget_ops_distribute_w(widget, ctx,
				                           grid->col_s[x].size,
				                           new_wh);
			}
		}
	}
}

static void distribute_h(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	unsigned int x, y;

	if (grid->flags & GP_WIDGET_GRID_UNIFORM)
		compute_rows_min_uniform_h(grid, ctx);
	else
		compute_rows_min_h(grid, ctx);

	unsigned int sum_row_fills = 0;

	/* cell fills */
	for (y = 0; y < grid->rows; y++)
		sum_row_fills += grid->row_s[y].fill;

	/* padding fills */
	for (y = 0; y <= grid->rows; y++)
		sum_row_fills += grid->row_b[y].fill;

	/* size to be distributed */
	unsigned int dy = self->h - self->min_h;

	/* Compute row offsets */
	if (sum_row_fills) {
		for (y = 0; y < grid->rows; y++)
			grid->row_s[y].size += dy * grid->row_s[y].fill / sum_row_fills;
	}

	/* Compute colum/row offsets */
	unsigned int cur_y = self->y + padd_size(ctx, grid->row_b[0].padd);
	if (sum_row_fills)
		cur_y += dy * grid->row_b[0].fill / sum_row_fills;

	for (y = 0; y < grid->rows; y++) {
		grid->row_s[y].off = cur_y;
		cur_y += grid->row_s[y].size + padd_size(ctx, grid->row_b[y+1].padd);
		if (sum_row_fills)
			cur_y += dy * grid->row_b[y+1].fill / sum_row_fills;
	}

	/* Place the widgets */
	for (y = 0; y < grid->rows; y++) {
		for (x = 0; x < grid->cols; x++) {
			gp_widget *widget = widget_grid_get(self, x, y);

			if (widget) {
				gp_widget_ops_distribute_h(widget, ctx,
				                           grid->row_s[y].size,
				                           new_wh);
			}
		}
	}
}

static void fill_padding(gp_widget *self, const gp_offset *offset,
                         const gp_widget_render_ctx *ctx)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	GP_DEBUG(3, "Filling grid %p padding", self);

	gp_coord x_off = self->x + offset->x;
	gp_coord y_off = self->y + offset->y;
	gp_coord end_y = y_off + self->h - 1;
	gp_coord end_x = x_off + self->w - 1;

	unsigned int y, cur_y = y_off;
	for (y = 0; y < grid->rows; y++) {
		gp_fill_rect_xyxy(ctx->buf, x_off, cur_y,
		                  end_x, offset->y + grid->row_s[y].off - 1,
				  ctx->bg_color);

		if (y < grid->rows)
			cur_y = grid->row_s[y].size + grid->row_s[y].off + offset->y;
	}

	gp_fill_rect_xyxy(ctx->buf, x_off, cur_y,
	                  end_x, end_y, ctx->bg_color);

	unsigned int x, cur_x = x_off;
	for (x = 0; x < grid->cols; x++) {
		if (grid->col_s[x].off) {
			gp_fill_rect_xyxy(ctx->buf, cur_x, y_off,
			                  offset->x + grid->col_s[x].off - 1, end_y,
					  ctx->bg_color);
		}

		if (x < grid->cols)
			cur_x = grid->col_s[x].off + grid->col_s[x].size + offset->x;
	}

	gp_fill_rect_xyxy(ctx->buf, cur_x, y_off,
		          end_x, end_y, ctx->bg_color);
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
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	gp_coord cur_x, cur_y;
	unsigned int x, y;

	if (gp_widget_should_redraw(self, flags)) {
		fill_padding(self, offset, ctx);
		gp_widget_ops_blit(ctx,
		                   self->x + offset->x, self->y + offset->y,
		                   self->w, self->h);
	}

	for (y = 0; y < grid->rows; y++) {
		cur_y = grid->row_s[y].off + offset->y;

		for (x = 0; x < grid->cols; x++) {
			cur_x = grid->col_s[x].off + offset->x;

			struct gp_widget *widget = widget_grid_get(self, x, y);

			if (!widget) {
				if (gp_widget_should_redraw(self, flags)) {
					gp_fill_rect_xywh(ctx->buf, cur_x, cur_y,
							  grid->col_s[x].size, grid->row_s[y].size,
							  ctx->debug_layout ? gp_widgets_color(ctx, GP_WIDGETS_COL_BLACK) : ctx->bg_color);
				}
				continue;
			}

			if (gp_widget_should_redraw(self, flags)) {
				fill_unused(widget, ctx, cur_x, cur_y,
				            grid->col_s[x].size, grid->row_s[y].size);
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

	if (grid->flags & GP_WIDGET_GRID_FRAME) {
		gp_rrect_xywh(ctx->buf, self->x + offset->x, self->y + offset->y,
			      self->w, self->h, ctx->text_color);
	}

	if (!ctx->debug_layout)
		return;

	static unsigned int depth = 0;

	int colors[] = {
		GP_WIDGETS_COL_BLACK,
		GP_WIDGETS_COL_CYAN,
		GP_WIDGETS_COL_MAGENTA,
	};

	gp_pixel col = gp_widgets_color(ctx, colors[depth++ % GP_ARRAY_SIZE(colors)]);

	unsigned int sx = grid->col_s[0].off + offset->x;
	unsigned int ex = grid->col_s[grid->cols-1].off + grid->col_s[grid->cols-1].size + offset->x;

	for (y = 0; y < grid->rows; y++) {
		gp_hline_xxy(ctx->buf, sx, ex, grid->row_s[y].off + offset->y, col);
		gp_hline_xxy(ctx->buf, sx, ex, grid->row_s[y].off + grid->row_s[y].size + offset->y, col);
	}

	unsigned int sy = grid->row_s[0].off + offset->y;
	unsigned int ey = grid->row_s[grid->rows-1].off + grid->row_s[grid->rows-1].size + offset->y;

	for (x = 0; x < grid->cols; x++) {
		gp_vline_xyy(ctx->buf, grid->col_s[x].off + offset->x, sy, ey, col);
		gp_vline_xyy(ctx->buf, grid->col_s[x].off + grid->col_s[x].size + offset->x, sy, ey, col);
	}
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
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	gp_widget *w = widget_grid_get(self, col, row);

	(void) sel;

	GP_DEBUG(4, "Trying to focus widget %p (%s) %ux%u",
		 w, gp_widget_type_id(w), col, row);

	if (!gp_widget_ops_render_focus(w, GP_FOCUS_IN))
		return 0;

	gp_widget_ops_render_focus(widget_grid_focused(self), GP_FOCUS_OUT);

	grid->focused_col = col;
	grid->focused_row = row;

	return 1;
}

static int focus_left(gp_widget *self, int sel)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	unsigned int col = grid->focused_col;
	unsigned int row = grid->focused_row;

	for (;;) {
		if (col == 0)
			return 0;

		if (try_focus(self, --col, row, sel))
			return 1;
	}
}

static int focus_right(gp_widget *self, int sel)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	unsigned int col = grid->focused_col;
	unsigned int row = grid->focused_row;

	for (;;) {
		if (++col >= grid->cols)
			return 0;

		if (try_focus(self, col, row, sel))
			return 1;
	}
}

static int focus_up(gp_widget *self, int sel)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	unsigned int col = grid->focused_col;
	unsigned int row = grid->focused_row;

	for (;;) {
		if (row == 0)
			return 0;

		if (try_focus(self, col, --row, sel))
			return 1;
	}
}

static int focus_down(gp_widget *self, int sel)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	unsigned int col = grid->focused_col;
	unsigned int row = grid->focused_row;

	for (;;) {
		if (++row >= grid->rows)
			return 0;

		if (try_focus(self, col, row, sel))
			return 1;
	}
}

static int focus_prev(gp_widget *self, int sel)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	unsigned int col = grid->focused_col;
	unsigned int row = grid->focused_row;

	for (;;) {
		if (col > 0) {
			col--;
		} else if (row > 0) {
			row--;
			col = grid->cols - 1;
		} else {
			return 0;
		}

		if (try_focus(self, col, row, sel))
			return 1;
	}
}

static int focus_next(gp_widget *self, int sel)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	unsigned int col = grid->focused_col;
	unsigned int row = grid->focused_row;

	for (;;) {
		if (col + 1 < grid->cols) {
			col++;
		} else if (row + 1 < grid->rows) {
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
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (!grid->focused)
		return grid->focused = focus_next(self, sel);

	return try_focus(self, grid->focused_col, grid->focused_row, sel);
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
                        struct gp_widget_grid_cell *cells,
                        unsigned int len)
{
	unsigned int i;

	if (coord < cells[0].off)
		return -1;

	//TODO interval divison?
	for (i = 0; i < len; i++) {
		if (coord >= cells[i].off && coord <= cells[i].off + cells[i].size)
			return i;
	}

	return -1;
}

static int focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                    unsigned int x, unsigned int y)
{
	int col, row;
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	col = coord_search(x + self->x, grid->col_s, grid->cols);
	row = coord_search(y + self->y, grid->row_s, grid->rows);

	if (col < 0 || row < 0)
		return 0;

	x -= grid->col_s[col].off - self->x;
	y -= grid->row_s[row].off - self->y;

	if (!gp_widget_ops_render_focus_xy(widget_grid_get(self, col, row), ctx, x, y))
		return 0;

	if (grid->focused_col != (unsigned int)col || grid->focused_row != (unsigned int)row)
		gp_widget_ops_render_focus(widget_grid_focused(self), GP_FOCUS_OUT);

	grid->focused_col = col;
	grid->focused_row = row;

	return 1;
}

static int focus_child(gp_widget *self, gp_widget *child)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	unsigned int col, row;

	if (!widget_grid_child_lookup(self, child, &col, &row))
		return 0;

	if (grid->focused_col == col && grid->focused_row == row)
		return 1;

	gp_widget_ops_render_focus(widget_grid_focused(self), GP_FOCUS_OUT);

	grid->focused_col = col;
	grid->focused_row = row;

	return 1;
}

static void set_border_padd(gp_widget *self, enum gp_widget_border border, uint8_t padd)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (border & GP_WIDGET_BORDER_LEFT)
		grid->col_b[0].padd = padd;

	if (border & GP_WIDGET_BORDER_RIGHT)
		grid->col_b[grid->cols].padd = padd;

	if (border & GP_WIDGET_BORDER_TOP)
		grid->row_b[0].padd = padd;

	if (border & GP_WIDGET_BORDER_BOTTOM)
		grid->row_b[grid->rows].padd = padd;

	if (!(border & GP_WIDGET_BORDER_CLEAR))
		return;

	border = ~border;

	if (border & GP_WIDGET_BORDER_LEFT)
		grid->col_b[0].padd = 0;

	if (border & GP_WIDGET_BORDER_RIGHT)
		grid->col_b[grid->cols].padd = 0;

	if (border & GP_WIDGET_BORDER_TOP)
		grid->row_b[0].padd = 0;

	if (border & GP_WIDGET_BORDER_BOTTOM)
		grid->row_b[grid->rows].padd = 0;
}

static void set_border_fill(gp_widget *self, enum gp_widget_border border, uint8_t fill)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (border & GP_WIDGET_BORDER_LEFT)
		grid->col_b[0].fill = fill;

	if (border & GP_WIDGET_BORDER_RIGHT)
		grid->col_b[grid->cols].fill = fill;

	if (border & GP_WIDGET_BORDER_TOP)
		grid->row_b[0].fill = fill;

	if (border & GP_WIDGET_BORDER_BOTTOM)
		grid->row_b[grid->rows].fill = fill;

	if (!(border & GP_WIDGET_BORDER_CLEAR))
		return;

	if ((~border) & GP_WIDGET_BORDER_LEFT)
		grid->col_b[0].fill = 0;

	if ((~border) & GP_WIDGET_BORDER_RIGHT)
		grid->col_b[grid->cols].fill = 0;

	if ((~border) & GP_WIDGET_BORDER_TOP)
		grid->row_b[0].fill = 0;

	if ((~border) & GP_WIDGET_BORDER_BOTTOM)
		grid->row_b[grid->rows].fill = 0;
}

static void set_rpad(gp_widget *self, uint8_t pad)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	unsigned int i;

	for (i = 1; i < grid->rows; i++)
		grid->row_b[i].padd = pad;
}

static void set_cpad(gp_widget *self, uint8_t pad)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	unsigned int i;

	for (i = 1; i < grid->cols; i++)
		grid->col_b[i].padd = pad;
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

static void parse_strarray(const char *sarray, void *array, unsigned int len,
                           void (*put)(void *array, unsigned int pos, uint8_t val),
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
			put(array, i++, val);
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

			put(array, i++, val);

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
			put(array, i++, val);
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

static void put_grid_cell_fill(void *array, unsigned int pos, uint8_t val)
{
	struct gp_widget_grid_cell *cell = array;

	cell[pos].fill = val;
}

static void put_grid_border_fill(void *array, unsigned int pos, uint8_t val)
{
	struct gp_widget_grid_gap *gap = array;

	gap[pos].fill = val;
}

static void put_grid_border_padd(void *array, unsigned int pos, uint8_t val)
{
	struct gp_widget_grid_gap *gap = array;

	gap[pos].padd = val;
}

static int alloc_grid(gp_widget **widget, struct gp_widget_grid **grid, unsigned int cols, unsigned int rows)
{
	if (*widget)
		return 0;

	*widget = gp_widget_grid_new(cols, rows, 0);
	if (!*widget)
		return 1;

	*grid = GP_WIDGET_PAYLOAD(*widget);

	return 0;
}

struct border {
	enum gp_widget_border border;
	int val;
};

static void parse_border(gp_json_reader *json, gp_json_val *val, struct border *border)
{
	if (val->type == GP_JSON_INT) {
		if (val->val_int < 0) {
			gp_json_warn(json, "Border must be >= 0!");
			return;
		}

		if (border->border == GP_WIDGET_BORDER_NONE)
			border->border = GP_WIDGET_BORDER_ALL;
		border->val = val->val_int;
		return;
	}

	if (val->type != GP_JSON_STR) {
		gp_json_warn(json, "Invalid type, expected int or string!");
		return;
	}

	if (!strcmp(val->val_str, "horiz")) {
		border->border = GP_WIDGET_BORDER_HORIZ | GP_WIDGET_BORDER_CLEAR;
		if (border->val == -1)
			border->val = 1;
	} else if (!strcmp(val->val_str, "vert")) {
		border->border = GP_WIDGET_BORDER_VERT | GP_WIDGET_BORDER_CLEAR;
		if (border->val == -1)
			border->val = 1;
	} else if (!strcmp(val->val_str, "none")) {
		border->border = GP_WIDGET_BORDER_ALL;
		border->val = 0;
	} else if (!strcmp(val->val_str, "all")) {
		border->border = GP_WIDGET_BORDER_ALL;
		if (border->val == -1)
			border->val = 1;
	} else if (!strcmp(val->val_str, "top")) {
		border->border = GP_WIDGET_BORDER_TOP | GP_WIDGET_BORDER_CLEAR;
		if (border->val == -1)
			border->val = 1;
	} else if (!strcmp(val->val_str, "bottom")) {
		border->border = GP_WIDGET_BORDER_BOTTOM | GP_WIDGET_BORDER_CLEAR;
		if (border->val == -1)
			border->val = 1;
	} else if (!strcmp(val->val_str, "left")) {
		border->border = GP_WIDGET_BORDER_LEFT | GP_WIDGET_BORDER_CLEAR;
		if (border->val == -1)
			border->val = 1;
	} else if (!strcmp(val->val_str, "right")) {
		border->border = GP_WIDGET_BORDER_RIGHT | GP_WIDGET_BORDER_CLEAR;
		if (border->val == -1)
			border->val = 1;
	} else {
		gp_json_warn(json, "Invalid border value!");
	}
}

enum keys {
	BORDER,
	CFILL,
	COLS,
	CPAD,
	CPADF,
	FRAME,
	PAD,
	RFILL,
	ROWS,
	RPAD,
	RPADF,
	UNIFORM,
	WIDGETS,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(BORDER, "border", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR_IDX(CFILL, "cfill", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(COLS, "cols", GP_JSON_INT),
	GP_JSON_OBJ_ATTR_IDX(CPAD, "cpad", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(CPADF, "cpadf", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(FRAME, "frame", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR_IDX(PAD, "pad", GP_JSON_INT),
	GP_JSON_OBJ_ATTR_IDX(RFILL, "rfill", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(ROWS,"rows", GP_JSON_INT),
	GP_JSON_OBJ_ATTR_IDX(RPAD, "rpad", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(RPADF, "rpadf", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(UNIFORM, "uniform", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR_IDX(WIDGETS, "widgets", GP_JSON_ARR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_grid(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	int cols = 1, rows = 1;
	gp_widget *ret = NULL;
	struct gp_widget_grid *grid = NULL;
	enum gp_widget_grid_flags flags = 0;
	struct border border = {.border = GP_WIDGET_BORDER_NONE, .val = -1};

	gp_json_reader_state obj_start = gp_json_reader_state_save(json);

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case BORDER:
			parse_border(json, val, &border);
		break;
		case CFILL:
			if (alloc_grid(&ret, &grid, cols, rows))
				goto skip;

			parse_strarray(val->val_str, grid->col_s, cols, put_grid_cell_fill, "Grid cfill");
		break;
		case COLS:
			if (val->val_int < 0) {
				gp_json_warn(json, "grid cols must be a positive number!");
				goto skip;
			}

			if (grid) {
				gp_json_warn(json, "grid cols must be set before widgets!");
				goto free_skip;
			}

			cols = val->val_int;
		break;
		case CPAD:
			if (alloc_grid(&ret, &grid, cols, rows))
				goto skip;

			parse_strarray(val->val_str, grid->col_b, cols+1, put_grid_border_padd, "Grid cpad");
		break;
		case CPADF:
			if (alloc_grid(&ret, &grid, cols, rows))
				goto skip;

			parse_strarray(val->val_str, grid->col_b, cols+1, put_grid_border_fill, "Grid cpadf");
		break;
		case FRAME:
			if (val->val_bool)
				flags |= GP_WIDGET_GRID_FRAME;
		break;
		case PAD:
			if (alloc_grid(&ret, &grid, cols, rows))
				goto skip;

			if (val->val_int < 0)
				gp_json_warn(json, "padd must be >= 0!");
			else
				set_pad(ret, val->val_int);
		break;
		case RFILL:
			if (alloc_grid(&ret, &grid, cols, rows))
				goto skip;

			parse_strarray(val->val_str, grid->row_s, rows, put_grid_cell_fill, "Grid rfill");
		break;
		case ROWS:
			if (val->val_int < 0) {
				gp_json_warn(json, "grid rows must be a positive number!");
				goto skip;
			}

			if (grid) {
				gp_json_warn(json, "grid rows must be set before widgets!");
				goto free_skip;
			}

			rows = val->val_int;
		break;
		case RPAD:
			if (alloc_grid(&ret, &grid, cols, rows))
				goto skip;

			parse_strarray(val->val_str, grid->row_b, rows+1, put_grid_border_padd, "Grid rpad");
		break;
		case RPADF:
			if (alloc_grid(&ret, &grid, cols, rows))
				goto skip;

			parse_strarray(val->val_str, grid->row_b, rows+1, put_grid_border_fill, "Grid rpadf");
		break;
		case UNIFORM:
			if (val->val_bool)
				flags |= GP_WIDGET_GRID_UNIFORM;
		break;
		case WIDGETS:
			if (alloc_grid(&ret, &grid, cols, rows))
				goto skip;

			if (val->type != GP_JSON_ARR) {
				gp_json_warn(json, "grid widgets must be an array!");
				goto skip;
			}

			int widgets = 0, warned = 0;

			GP_JSON_ARR_FOREACH(json, val) {
				if (widgets >= cols * rows) {
					if (!warned) {
						warned = 1;
						gp_json_warn(json, "Too many widgets!");
					}
					gp_json_obj_skip(json);
					continue;
				}

				gp_widget *child = gp_widget_from_json(json, val, ctx);
				gp_widget_grid_put(ret, widgets/rows, widgets%rows, child);

				widgets++;
			}

			if (widgets < cols * rows)
				gp_json_warn(json, "Not enough widgets to fill grid!");
		break;
		}
	}

	if (alloc_grid(&ret, &grid, cols, rows))
		goto skip;

	grid->flags = flags;

	gp_widget_grid_border_set(ret, border.border, border.val, -1);

	return ret;
free_skip:
	gp_widget_free(ret);
skip:
	gp_json_reader_state_load(json, obj_start);
	gp_json_obj_skip(json);
	return NULL;
}

static void free_(gp_widget *self)
{
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	gp_matrix_free(grid->widgets);

	gp_vec_free(grid->col_b);
	gp_vec_free(grid->row_b);

	gp_vec_free(grid->col_s);
	gp_vec_free(grid->row_s);
}

static void for_each_child(gp_widget *self, void (*func)(gp_widget *child))
{
	unsigned int x, y;
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

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
	.focus_child = focus_child,
	.distribute_w = distribute_w,
	.distribute_h = distribute_h,
	.for_each_child = for_each_child,
	.from_json = json_to_grid,
	.id = "grid",
};

static void init_rows(struct gp_widget_grid *grid, unsigned int row, unsigned int rows, int flag)
{
	unsigned int i;

	for (i = row; i < row + rows + flag; i++)
		grid->row_b[i].padd = 1;

	for (i = row; i < row + rows; i++)
		grid->row_s[i].fill = 1;
}

static void init_cols(struct gp_widget_grid *grid, unsigned int col, unsigned int cols, int flag)
{
	unsigned int i;

	for (i = col; i < cols + flag; i++)
		grid->col_b[i].padd = 1;

	for (i = col; i < cols; i++)
		grid->col_s[i].fill = 1;
}

gp_widget *gp_widget_grid_new(unsigned int cols, unsigned int rows,
                              enum gp_widget_grid_flags flags)
{
	gp_widget *ret;

	if (flags & ~(GP_WIDGET_GRID_FRAME | GP_WIDGET_GRID_UNIFORM)) {
		GP_WARN("Invalid flags 0x%x", flags);
		return NULL;
	}

	ret = gp_widget_new(GP_WIDGET_GRID, GP_WIDGET_CLASS_NONE, sizeof(struct gp_widget_grid));
	if (!ret)
		return NULL;

	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(ret);

	grid->flags = flags;

	grid->cols = cols;
	grid->rows = rows;

	grid->widgets = gp_matrix_new(cols, rows, sizeof(gp_widget*));

	grid->col_s = gp_vec_new(cols, sizeof(struct gp_widget_grid_cell));
	grid->row_s = gp_vec_new(rows, sizeof(struct gp_widget_grid_cell));

	grid->col_b = gp_vec_new(cols + 1, sizeof(struct gp_widget_grid_gap));
	grid->row_b = gp_vec_new(rows + 1, sizeof(struct gp_widget_grid_gap));

	init_cols(grid, 0, cols, 1);
	init_rows(grid, 0, rows, 1);

	return ret;
}

unsigned int gp_widget_grid_cols_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, 0);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	return grid->cols;
}

unsigned int gp_widget_grid_rows_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, 0);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	return grid->rows;
}

enum gp_widget_grid_flags gp_widget_grid_flags_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, 0);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	return grid->flags;
}

void gp_widget_grid_flags_set(gp_widget *self, enum gp_widget_grid_flags flags)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (flags & ~(GP_WIDGET_GRID_FRAME | GP_WIDGET_GRID_UNIFORM)) {
		GP_WARN("Invalid flags 0x%x", flags);
		return;
	}

	grid->flags = flags;

	gp_widget_resize(self);
}

const gp_widget_grid_gap *gp_widget_grid_cols_gaps_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, 0);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	return grid->col_b;
}

const gp_widget_grid_gap *gp_widget_grid_rows_gaps_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, 0);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	return grid->row_b;
}

static int assert_col_row(gp_widget *self, unsigned int col, unsigned int row)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, 0);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (col >= grid->cols) {
		GP_BUG("Invalid column index %u Grid %p %ux%u",
			col, self, grid->cols, grid->rows);
		return 1;
	}

	if (row >= grid->rows) {
		GP_BUG("Invalid row index %u Grid %p %ux%u",
			row, self, grid->cols, grid->rows);
		return 1;
	}

	return 0;
}

gp_widget *gp_widget_grid_put(gp_widget *self, unsigned int col, unsigned int row,
                              gp_widget *child)
{
	gp_widget *ret;

	if (!child)
		return gp_widget_grid_rem(self, col, row);

	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, NULL);

	if (assert_col_row(self, col, row))
		return NULL;

	ret = widget_grid_put(self, child, col, row);
	if (ret)
		ret->parent = NULL;

	gp_widget_resize(self);
	gp_widget_redraw(child);

	return ret;
}

void gp_widget_grid_rows_ins(gp_widget *self, unsigned int row, unsigned int rows)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (row > grid->rows) {
		GP_WARN("Row %u out of grid (%p rows %u)", row, self, grid->rows);
		return;
	}

	grid->widgets = gp_matrix_rows_ins(grid->widgets, grid->cols, grid->rows, row, rows);

	grid->row_s = gp_vec_ins(grid->row_s, row, rows);
	grid->row_b = gp_vec_ins(grid->row_b, row, rows);

	init_rows(grid, row, rows, 0);

	grid->rows += rows;

	gp_widget_resize(self);
}

unsigned int gp_widget_grid_rows_append(gp_widget *self, unsigned int rows)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, (unsigned int)-1);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	unsigned int ret = grid->rows;

	gp_widget_grid_rows_ins(self, grid->rows, rows);

	return ret;
}

void gp_widget_grid_rows_prepend(gp_widget *self, unsigned int rows)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );

	gp_widget_grid_rows_ins(self, 0, rows);
}

void gp_widget_grid_rows_del(gp_widget *self, unsigned int row, unsigned int rows)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	size_t r, c;

	if (row >= grid->rows) {
		GP_WARN("Row %u out of grid (%p rows %u)", row, self, grid->rows);
		return;
	}

	if (row + rows > grid->rows) {
		GP_WARN("Block %u at row %u out of grid (%p rows %u)",
		        rows, row, self, grid->rows);
		return;
	}

	for (r = 0; r < rows; r++) {
		for (c = 0; c < grid->cols; c++)
			gp_widget_free(grid->widgets[gp_matrix_idx(grid->rows, c, r + row)]);
	}

	grid->widgets = gp_matrix_rows_del(grid->widgets, grid->cols, grid->rows, row, rows);

	grid->row_s = gp_vec_del(grid->row_s, row, rows);
	grid->row_b = gp_vec_del(grid->row_b, row, rows);

	grid->rows -= rows;

	gp_widget_resize(self);
	gp_widget_redraw(self);
}

void gp_widget_grid_cols_ins(gp_widget *self, unsigned int col, unsigned int cols)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (col > grid->cols) {
		GP_WARN("Col %u out of grid (%p cols %u)", col, self, grid->cols);
		return;
	}

	grid->widgets = gp_matrix_cols_ins(grid->widgets, grid->rows, col, cols);

	grid->col_s = gp_vec_ins(grid->col_s, col, cols);
	grid->col_b = gp_vec_ins(grid->col_b, col, cols);

	init_cols(grid, col, cols, 0);

	grid->cols += cols;

	gp_widget_resize(self);
}

unsigned int gp_widget_grid_cols_append(gp_widget *self, unsigned int cols)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, (unsigned int)-1);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	unsigned int ret = grid->cols;

	gp_widget_grid_cols_ins(self, grid->cols, cols);

	return ret;
}

void gp_widget_grid_cols_prepend(gp_widget *self, unsigned int cols)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );

	gp_widget_grid_cols_ins(self, 0, cols);
}

void gp_widget_grid_cols_del(gp_widget *self, unsigned int col, unsigned int cols)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	size_t r, c;

	if (col >= grid->cols) {
		GP_WARN("Col %u out of grid (%p cols %u)", col, self, grid->cols);
		return;
	}

	if (col + cols > grid->cols) {
		GP_WARN("Block %u at col %u out of grid (%p cols %u)",
		        cols, col, self, grid->cols);
		return;
	}

	for (c = 0; c < cols; c++) {
		for (r = 0; r < grid->rows; r++)
			gp_widget_free(grid->widgets[gp_matrix_idx(grid->rows, c + col, r)]);
	}

	grid->widgets = gp_matrix_cols_del(grid->widgets, grid->rows, col, cols);

	grid->col_s = gp_vec_del(grid->col_s, col, cols);
	grid->col_b = gp_vec_del(grid->col_b, col, cols);

	grid->cols -= cols;

	gp_widget_resize(self);
	gp_widget_redraw(self);
}

gp_widget *gp_widget_grid_rem(gp_widget *self, unsigned int col, unsigned int row)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, NULL);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	gp_widget *ret;

	if (assert_col_row(self, col, row))
		return NULL;

	ret = widget_grid_put(self, NULL, col, row);
	if (ret)
		ret->parent = NULL;

	if (grid->focused_col == col && grid->focused_row == row) {
		grid->focused_col = 0;
		grid->focused_row = 0;
		grid->focused = 0;
	}

	gp_widget_resize(self);
	gp_widget_redraw(self);

	return ret;
}

gp_widget *gp_widget_grid_get(gp_widget *self, unsigned int col, unsigned int row)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, NULL);

	if (assert_col_row(self, col, row))
		return NULL;

	return widget_grid_get(self, col, row);
}

void gp_widget_grid_border_set(gp_widget *self, enum gp_widget_border border,
                               int padd, int fill)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );

	padd = GP_MIN(padd, (int)UINT8_MAX);
	fill = GP_MIN(fill, (int)UINT8_MAX);

	if (padd >= 0)
		set_border_padd(self, border, padd);

	if (fill >= 0)
		set_border_fill(self, border, fill);
}

void gp_widget_grid_cpad_set(gp_widget *self, unsigned int col, int padd, int fill)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	padd = GP_MIN(padd, (int)UINT8_MAX);
	fill = GP_MIN(fill, (int)UINT8_MAX);

	if (col == GP_WIDGET_GRID_IPADD_ALL) {
		unsigned int c;

		for (c = 1; c < grid->cols; c++) {
			if (padd >= 0)
				grid->col_b[c].padd = padd;
			if (fill >= 0)
				grid->col_b[c].fill = fill;
		}
		gp_widget_resize(self);
		return;
	}

	if (col > grid->cols) {
		GP_WARN("Invalid col padding index %u > cols=%u", col, grid->cols);
		return;
	}

	if (padd >= 0)
		grid->col_b[col].padd = padd;
	if (fill >= 0)
		grid->col_b[col].fill = fill;

	gp_widget_resize(self);
}

void gp_widget_grid_rpad_set(gp_widget *self, unsigned int row, int padd, int fill)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	padd = GP_MIN(padd, (int)UINT8_MAX);
	fill = GP_MIN(fill, (int)UINT8_MAX);

	if (row == GP_WIDGET_GRID_IPADD_ALL) {
		unsigned int r;

		for (r = 1; r < grid->rows; r++) {
			if (padd >= 0)
				grid->row_b[r].padd = padd;
			if (fill >= 0)
				grid->row_b[r].fill = fill;
		}
		gp_widget_resize(self);
		return;
	}

	if (row > grid->rows) {
		GP_WARN("Invalid row padding index %u > rows=%u", row, grid->rows);
		return;
	}

	if (padd >= 0)
		grid->row_b[row].padd = padd;
	if (fill >= 0)
		grid->row_b[row].fill = fill;

	gp_widget_resize(self);
}

void gp_widget_grid_col_fill_set(gp_widget *self, unsigned int col, uint8_t fill)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (col >= grid->cols) {
		GP_WARN("Invalid grid col %u have %u cols", col, grid->cols);
		return;
	}

	grid->col_s[col].fill = fill;
}

void gp_widget_grid_row_fill_set(gp_widget *self, unsigned int row, uint8_t fill)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, );
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);

	if (row >= grid->rows) {
		GP_WARN("Invalid grid row %u have %u rows", row, grid->rows);
		return;
	}

	grid->row_s[row].fill = fill;
}

/*

 This part implements hbox and vbox which are JSON shortcuts for a grid with single col/row.

 */

static int get_padd_fill(gp_json_reader *json, gp_json_val *val,
                         unsigned int *padd, unsigned int *fill)
{
	int cnt = 0;

	GP_JSON_ARR_FOREACH(json, val) {
		switch (val->type) {
		case GP_JSON_INT:
			switch (cnt) {
			case 0:
				*padd = val->val_int;
				cnt++;
			break;
			case 1:
				*fill = val->val_int;
				cnt++;
			break;
			default:
				goto err;
			}
		break;
		default:
			goto err;
		}
	}

	if (cnt == 2)
		return 1;

err:
	gp_json_warn(json, "Invalid [padd, fill] tupple");
	return 0;
}

static gp_widget *get_widgets(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx, int horiz)
{
	unsigned int cnt = 0;
	gp_widget *ret, *tmp;

	gp_json_reader_state arr_start = gp_json_reader_state_save(json);

	GP_JSON_ARR_FOREACH(json, val) {
		switch (val->type) {
		case GP_JSON_OBJ:
			cnt++;
			gp_json_obj_skip(json);
		break;
		case GP_JSON_ARR:
			gp_json_arr_skip(json);
		break;
		default:
			gp_json_warn(json, "Invalid hbox element");
		}
	}

	uint32_t cols = horiz ? cnt : 1;
	uint32_t rows = horiz ? 1 : cnt;

	ret = gp_widget_grid_new(cols, rows, 0);
	if (!ret)
		return NULL;

	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(ret);

	gp_json_reader_state_load(json, arr_start);
	cnt = 0;
	unsigned int padd, fill;

	struct gp_widget_grid_gap *gap = horiz ? grid->col_b : grid->row_b;

	GP_JSON_ARR_FOREACH(json, val) {
		switch (val->type) {
		case GP_JSON_OBJ:
			tmp = gp_widget_from_json(json, val, ctx);

			if (horiz)
				gp_widget_grid_put(ret, cnt, 0, tmp);
			else
				gp_widget_grid_put(ret, 0, cnt, tmp);

			cnt++;
		break;
		case GP_JSON_ARR:
			if (get_padd_fill(json, val, &padd, &fill)) {
				gap[cnt].padd = padd;
				gap[cnt].fill = fill;
			}
		break;
		default:
		break;
		}
	}

	return ret;
}

enum box_keys {
	BOX_BORDER,
	BOX_CELL_FILL,
	BOX_FRAME,
	BOX_PADD,
	BOX_PADD_FILL,
	BOX_UNIFORM,
	BOX_WIDGETS,
};

static const gp_json_obj_attr box_attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(BOX_BORDER, "border", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR_IDX(BOX_CELL_FILL, "cell-fill", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(BOX_FRAME, "frame", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR_IDX(BOX_PADD, "padd", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(BOX_PADD_FILL, "padd-fill", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(BOX_UNIFORM, "uniform", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR_IDX(BOX_WIDGETS, "widgets", GP_JSON_ARR),
};

static const gp_json_obj box_obj_filter = {
	.attrs = box_attrs,
	.attr_cnt = GP_ARRAY_SIZE(box_attrs),
};

static gp_widget *box_from_json(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx, int horiz)
{
	struct border border = {.border = GP_WIDGET_BORDER_NONE, .val = -1};
	enum gp_widget_grid_flags flags = 0;
	gp_widget *ret = NULL;
	struct gp_widget_grid *grid = NULL;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &box_obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case BOX_BORDER:
			parse_border(json, val, &border);
		break;
		case BOX_CELL_FILL:
			if (!ret) {
				gp_json_warn(json, "fill before widgets array!");
				continue;
			}
			if (horiz)
				parse_strarray(val->val_str, grid->col_s, grid->cols, put_grid_cell_fill, "Cell fill");
			else
				parse_strarray(val->val_str, grid->row_s, grid->rows, put_grid_cell_fill, "Cell fill");
		break;
		case BOX_FRAME:
			if (val->val_bool)
				flags |= GP_WIDGET_GRID_FRAME;
		break;
		case BOX_PADD:
			if (!ret) {
				gp_json_warn(json, "fill before widgets array!");
				continue;
			}
			if (horiz)
				parse_strarray(val->val_str, grid->col_b, grid->cols+1, put_grid_border_padd, "Padd");
			else
				parse_strarray(val->val_str, grid->row_b, grid->rows+1, put_grid_border_padd, "Padd");
		break;
		case BOX_PADD_FILL:
			if (!ret) {
				gp_json_warn(json, "fill before widgets array!");
				continue;
			}
			if (horiz)
				parse_strarray(val->val_str, grid->col_b, grid->cols+1, put_grid_border_fill, "Padd fill");
			else
				parse_strarray(val->val_str, grid->row_b, grid->rows+1, put_grid_border_fill, "Padd fill");
		break;
		case BOX_UNIFORM:
			if (val->val_bool)
				flags |= GP_WIDGET_GRID_UNIFORM;
		break;
		case BOX_WIDGETS:
			ret = get_widgets(json, val, ctx, horiz);
			grid = GP_WIDGET_PAYLOAD(ret);
		break;
		}
	}

	if (!ret)
		return NULL;

	grid->flags = flags;

	gp_widget_grid_border_set(ret, border.border, border.val, -1);

	return ret;
}

__attribute__((visibility ("hidden")))
gp_widget *hbox_from_json(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	return box_from_json(json, val, ctx, 1);
}

__attribute__((visibility ("hidden")))
gp_widget *vbox_from_json(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	return box_from_json(json, val, ctx, 0);
}
