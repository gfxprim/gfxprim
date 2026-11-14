//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <math.h>
#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_json.h>

struct gp_widget_graph {
	gp_widget_size min_w;
	gp_widget_size min_h;

	char *x_label;
	char *y_label;

	long min_y_fixed:1;
	long max_y_fixed:1;

	/* Graph scaling window */
	double min_x;
	double max_x;
	double min_y;
	double max_y;

	/* Graph data */
	enum gp_widget_graph_style graph_style;
	enum gp_widget_graph_flags flags;
	enum gp_widgets_color color;
	gp_cbuffer data_idx;
	struct gp_widget_graph_point *data;
};

/* Height of the strip below the graph with the x label */
static gp_size x_label_h(struct gp_widget_graph *graph, const gp_widget_render_ctx *ctx)
{
	if (!graph->x_label)
		return 0;

	return gp_text_height(ctx->font) + ctx->padd;
}

/*
 * The y label is drawn in the top left corner of the graph, the data are drawn
 * below it so that they are never covered by the label.
 */
static gp_size y_label_h(struct gp_widget_graph *graph, const gp_widget_render_ctx *ctx)
{
	if (!graph->y_label)
		return 0;

	return gp_text_height(ctx->font) + ctx->padd;
}

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);
	unsigned int ret = gp_widget_size_units_get(&graph->min_w, ctx);

	if (graph->x_label)
		ret = GP_MAX(ret, gp_text_wbbox(ctx->font, graph->x_label));

	if (graph->y_label)
		ret = GP_MAX(ret, gp_text_wbbox(ctx->font, graph->y_label) + ctx->padd);

	return ret;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);
	unsigned int ret = gp_widget_size_units_get(&graph->min_h, ctx);

	return ret + x_label_h(graph, ctx) + y_label_h(graph, ctx);
}

static gp_coord transform_x(struct gp_widget_graph *graph, double x, gp_coord w)
{
	double range = graph->max_x - graph->min_x;

	return round(w * fabs(x - graph->min_x) / range);
}

static gp_coord transform_y(struct gp_widget_graph *graph, double y, gp_coord h)
{
	double range = graph->max_y - graph->min_y;

	/* Only a range fixed by the user to a single value can be empty */
	if (range == 0)
		return h/2;

	return h - round(h * fabs(y - graph->min_y) / range);
}

static void render_line_graph(struct gp_widget_graph *graph,
                              gp_pixmap *pix, gp_coord w, gp_coord h,
                              const gp_widget_render_ctx *ctx)
{
	gp_cbuffer_iter iter;

	gp_size line_th = (ctx->fr_thick+1)/2;
	gp_size circle_r = line_th;
	gp_pixel col = ctx->colors[graph->color];

	size_t first = gp_cbuffer_first(&graph->data_idx);

	gp_coord px = transform_x(graph, graph->data[first].x, w-2*circle_r-1) + circle_r;
	gp_coord py = transform_y(graph, graph->data[first].y, h-2*circle_r-1) + circle_r;

	GP_CBUFFER_FORRANGE(&graph->data_idx, &iter, 1, gp_cbuffer_used(&graph->data_idx)-1) {
		gp_coord dx = transform_x(graph, graph->data[iter.idx].x, w-2*circle_r-1) + circle_r;
		gp_coord dy = transform_y(graph, graph->data[iter.idx].y, h-2*circle_r-1) + circle_r;

		gp_line_th(pix, px, py, dx, dy, line_th, col);

		px = dx; py = dy;
	}

	GP_CBUFFER_FOREACH(&graph->data_idx, &iter) {
		gp_coord dx = transform_x(graph, graph->data[iter.idx].x, w-2*circle_r-1) + circle_r;
		gp_coord dy = transform_y(graph, graph->data[iter.idx].y, h-2*circle_r-1) + circle_r;

		gp_fill_circle(pix, dx, dy, circle_r, col);
	}
}


static void render_point_graph(struct gp_widget_graph *graph,
                               gp_pixmap *pix, gp_coord w, gp_coord h,
                               const gp_widget_render_ctx *ctx)
{
	gp_cbuffer_iter iter;

	gp_size line_th = (ctx->fr_thick+1)/2;
	gp_size circle_r = line_th*2;
	gp_pixel col = ctx->colors[graph->color];

	GP_CBUFFER_FOREACH(&graph->data_idx, &iter) {
		gp_coord dx = transform_x(graph, graph->data[iter.idx].x, w-2*circle_r-1) + circle_r;
		gp_coord dy = transform_y(graph, graph->data[iter.idx].y, h-2*circle_r-1) + circle_r;

		gp_fill_circle(pix, dx, dy, circle_r, col);
	}
}

static void render_fill_graph(struct gp_widget_graph *graph,
                              gp_pixmap *pix, gp_size w, gp_size h,
                              const gp_widget_render_ctx *ctx)
{
	gp_coord poly[gp_cbuffer_used(&graph->data_idx) * 2 + 4];
	gp_pixel col = ctx->colors[graph->color];
	gp_cbuffer_iter iter;
	size_t pos = 1;

	poly[0] = 0;
	poly[1] = h - 1;

	GP_CBUFFER_FOREACH(&graph->data_idx, &iter) {
		poly[2 * pos] = transform_x(graph, graph->data[iter.idx].x, w - 1);
		poly[2 * pos + 1] = transform_y(graph, graph->data[iter.idx].y, h - 1);
		pos++;
	}

	poly[2 * pos] = w - 1;
	poly[2 * pos + 1] = h - 1;

	pos++;

	gp_fill_polygon(pix, 0, 0, pos, poly, col);
}

static double clamp_y(struct gp_widget_graph *graph, double y)
{
	return GP_MAX(graph->min_y, GP_MIN(graph->max_y, y));
}

/*
 * Fills a rectangle, the part that overlaps rows by0 to by1 (inclusive) is
 * filled with the in color, the rest with the out color.
 */
static void fill_rect_split(gp_pixmap *pix, gp_coord x0, gp_coord y0,
                            gp_coord x1, gp_coord y1, gp_coord by0, gp_coord by1,
                            gp_pixel in, gp_pixel out)
{
	gp_coord iy0 = GP_MAX(y0, by0);
	gp_coord iy1 = GP_MIN(y1, by1);

	if (iy0 > iy1) {
		gp_fill_rect_xyxy(pix, x0, y0, x1, y1, out);
		return;
	}

	if (y0 < iy0)
		gp_fill_rect_xyxy(pix, x0, y0, x1, iy0 - 1, out);

	gp_fill_rect_xyxy(pix, x0, iy0, x1, iy1, in);

	if (iy1 < y1)
		gp_fill_rect_xyxy(pix, x0, iy1 + 1, x1, y1, out);
}

/*
 * Draws an error bar centered at cx, the parts that overlap the bar rows by0
 * to by1 are drawn in the background color so that they stay visible.
 */
static void render_err_bar(gp_pixmap *pix, gp_coord cx, gp_coord y0, gp_coord y1,
                           gp_coord cap_w, gp_size th, gp_coord by0, gp_coord by1,
                           gp_pixel col, gp_pixel bg)
{
	gp_coord lx = cx - (gp_coord)th/2;
	gp_coord rx = lx + th - 1;

	/* vertical line */
	fill_rect_split(pix, lx, y0, rx, y1, by0, by1, bg, col);

	/* caps, these do not extend over the error bar ends */
	fill_rect_split(pix, cx - cap_w, y0, cx + cap_w, GP_MIN(y0 + (gp_coord)th - 1, y1),
	                by0, by1, bg, col);
	fill_rect_split(pix, cx - cap_w, GP_MAX(y1 - (gp_coord)th + 1, y0), cx + cap_w, y1,
	                by0, by1, bg, col);
}

/*
 * Bar like graphs split the width into equal slots, one for each point, the
 * x coordinate is not used for positioning.
 */
static void slot_get(size_t i, size_t cnt, gp_size w, gp_coord *x0, gp_coord *x1)
{
	*x0 = i * w / cnt;
	*x1 = (i + 1) * w / cnt - 1;
}

/*
 * Transforms a y value clamped to the range into widget area with a margin on
 * the top and bottom.
 */
static gp_coord transform_y_m(struct gp_widget_graph *graph, double y,
                              gp_size h, gp_size margin)
{
	return transform_y(graph, clamp_y(graph, y), h - 1 - 2 * margin) + margin;
}

static int draw_err_bar(struct gp_widget_graph *graph,
                        struct gp_widget_graph_point *point)
{
	return (graph->flags & GP_WIDGET_GRAPH_ERROR_BARS) && point->err != 0;
}

static void render_bar_graph(struct gp_widget_graph *graph,
                             gp_pixmap *pix, gp_size w, gp_size h,
                             const gp_widget_render_ctx *ctx)
{
	size_t i = 0, cnt = gp_cbuffer_used(&graph->data_idx);
	gp_pixel col = ctx->colors[graph->color];
	gp_size th = (ctx->fr_thick+1)/2;
	double base = clamp_y(graph, 0);
	gp_coord base_y = transform_y_m(graph, base, h, 0);
	gp_cbuffer_iter iter;

	/* Nothing to draw, e.g. all values in autorange mode are zero */
	if (graph->max_y == graph->min_y)
		return;

	GP_CBUFFER_FOREACH(&graph->data_idx, &iter) {
		struct gp_widget_graph_point *point = &graph->data[iter.idx];
		gp_coord x0, x1;
		double val = clamp_y(graph, point->y);
		gp_coord bar_y = transform_y_m(graph, val, h, 0);

		slot_get(i++, cnt, w, &x0, &x1);

		gp_coord slot = x1 - x0 + 1;
		gp_coord gap = slot > 2 ? GP_MAX(1, slot/5) : 0;

		x0 += gap/2;
		x1 -= gap - gap/2;

		if (val != base)
			gp_fill_rect_xyxy(pix, x0, base_y, x1, bar_y, col);

		if (!draw_err_bar(graph, point))
			continue;

		double err = fabs(point->err);
		gp_coord err_y0 = transform_y_m(graph, point->y + err, h, 0);
		gp_coord err_y1 = transform_y_m(graph, point->y - err, h, 0);
		gp_coord by0 = GP_MIN(base_y, bar_y);
		gp_coord by1 = GP_MAX(base_y, bar_y);

		/* No bar, nothing to keep visible against */
		if (val == base)
			by0 = by1 + 1;

		render_err_bar(pix, (x0 + x1)/2, err_y0, err_y1,
		               GP_MAX((x1 - x0)/4, (gp_coord)th), th,
		               by0, by1, col, ctx->fg_color);
	}
}

static void render_stem_graph(struct gp_widget_graph *graph,
                              gp_pixmap *pix, gp_size w, gp_size h,
                              const gp_widget_render_ctx *ctx)
{
	size_t i = 0, cnt = gp_cbuffer_used(&graph->data_idx);
	gp_pixel col = ctx->colors[graph->color];
	gp_size th = (ctx->fr_thick+1)/2;
	gp_coord slot = w / cnt;
	/* The dot has to fit into the slot */
	gp_size r = GP_MIN(2 * th, (gp_size)GP_MAX(slot/2 - 1, 1));
	gp_coord base_y = transform_y_m(graph, 0, h, r);
	gp_cbuffer_iter iter;

	GP_CBUFFER_FOREACH(&graph->data_idx, &iter) {
		struct gp_widget_graph_point *point = &graph->data[iter.idx];
		gp_coord x0, x1;
		gp_coord val_y = transform_y_m(graph, point->y, h, r);

		slot_get(i++, cnt, w, &x0, &x1);

		gp_coord cx = (x0 + x1)/2;
		gp_coord lx = cx - (gp_coord)th/2;

		gp_fill_rect_xyxy(pix, lx, base_y, lx + th - 1, val_y, col);

		if (draw_err_bar(graph, point)) {
			double err = fabs(point->err);
			gp_coord err_y0 = transform_y_m(graph, point->y + err, h, r);
			gp_coord err_y1 = transform_y_m(graph, point->y - err, h, r);
			gp_coord cap_w = GP_MAX(GP_MIN(2 * (gp_coord)r, (x1 - x0)/2 - 1), (gp_coord)th);

			/* No bar to keep the error bar visible against */
			render_err_bar(pix, cx, err_y0, err_y1, cap_w, th,
			               1, 0, col, col);
		}

		gp_fill_circle(pix, cx, val_y, r, col);
	}
}

static void render_step_graph(struct gp_widget_graph *graph,
                              gp_pixmap *pix, gp_size w, gp_size h,
                              const gp_widget_render_ctx *ctx)
{
	size_t i = 0, cnt = gp_cbuffer_used(&graph->data_idx);
	gp_pixel col = ctx->colors[graph->color];
	gp_size th = (ctx->fr_thick+1)/2;
	gp_cbuffer_iter iter;
	gp_coord prev_y = 0;

	GP_CBUFFER_FOREACH(&graph->data_idx, &iter) {
		/* Top row of the line, the line is th pixels thick */
		gp_coord y = transform_y(graph, clamp_y(graph, graph->data[iter.idx].y), h - th);
		gp_coord x0, x1;

		slot_get(i, cnt, w, &x0, &x1);

		/* Vertical jump at the boundary from the previous value */
		if (i) {
			gp_coord lx = x0 - (gp_coord)th/2;

			gp_fill_rect_xyxy(pix, lx, GP_MIN(prev_y, y),
			                  lx + th - 1, GP_MAX(prev_y, y) + th - 1, col);
		}

		/* Horizontal line over the whole slot */
		gp_fill_rect_xyxy(pix, x0, y, x1, y + th - 1, col);

		prev_y = y;
		i++;
	}
}

static void render_graph(struct gp_widget_graph *graph, gp_pixmap *pix,
                         gp_size w, gp_size h, const gp_widget_render_ctx *ctx)
{
	switch (graph->graph_style) {
	case GP_WIDGET_GRAPH_POINT:
		render_point_graph(graph, pix, w, h, ctx);
	break;
	case GP_WIDGET_GRAPH_LINE:
		render_line_graph(graph, pix, w, h, ctx);
	break;
	case GP_WIDGET_GRAPH_FILL:
		render_fill_graph(graph, pix, w, h, ctx);
	break;
	case GP_WIDGET_GRAPH_BAR:
		render_bar_graph(graph, pix, w, h, ctx);
	break;
	case GP_WIDGET_GRAPH_STEM:
		render_stem_graph(graph, pix, w, h, ctx);
	break;
	case GP_WIDGET_GRAPH_STEP:
		render_step_graph(graph, pix, w, h, ctx);
	break;
	default:
		GP_WARN("Invalid graph type %i", graph->graph_style);
	}
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);
	gp_coord x = self->x + offset->x;
	gp_coord y = self->y + offset->y;
	gp_size w = self->w;
	gp_size h = self->h;
	gp_pixmap pix;

	(void) flags;

	gp_fill_rect_xywh(ctx->buf, x, y, w, h, ctx->fg_color);
        gp_widget_ops_blit(ctx, x, y, w, h);

	if (graph->x_label) {
		gp_size lh = x_label_h(graph, ctx);

		h -= lh;

		gp_text(ctx->buf, ctx->font, x + w/2, y + h + ctx->padd,
		        GP_ALIGN_CENTER | GP_VALIGN_BELOW,
		        ctx->text_color, ctx->fg_color, graph->x_label);
	}

	if (graph->y_label) {
		gp_size lh = y_label_h(graph, ctx);

		gp_text(ctx->buf, ctx->font, x + ctx->padd, y,
		        GP_ALIGN_RIGHT | GP_VALIGN_BELOW,
		        ctx->text_color, ctx->fg_color, graph->y_label);

		y += lh;
		h -= lh;
	}

	if (!gp_cbuffer_used(&graph->data_idx))
		return;

	gp_sub_pixmap(ctx->buf, &pix, x, y, w, h);

	render_graph(graph, &pix, w, h, ctx);
}

const char *gp_widget_graph_style_names[GP_WIDGET_GRAPH_STYLE_MAX] = {
	"point",
	"line",
	"fill",
	"bar",
	"stem",
	"step",
};

enum keys {
	COLOR,
	ERROR_BARS,
	MAX_DATA_POINTS,
	MIN_H,
	MIN_W,
	STYLE,
	X_LABEL,
	Y_LABEL,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(COLOR, "color", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(ERROR_BARS, "error_bars", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR_IDX(MAX_DATA_POINTS, "max_data_points", GP_JSON_INT),
	GP_JSON_OBJ_ATTR_IDX(MIN_H, "min_h", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR_IDX(MIN_W, "min_w", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR_IDX(STYLE, "style", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(X_LABEL, "x_label", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(Y_LABEL, "y_label", GP_JSON_STR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static enum gp_widget_graph_style str_to_style(const char *style)
{
	size_t i;

	for (i = 0; i < GP_WIDGET_GRAPH_STYLE_MAX; i++) {
		if (!strcmp(style, gp_widget_graph_style_names[i]))
			return i;
	}

	return GP_WIDGET_GRAPH_STYLE_MAX;
}

static gp_widget *json_to_graph(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	gp_widget_size w = {};
	gp_widget_size h = {};
	size_t data_points = 100;
	enum gp_widget_graph_style style = GP_WIDGET_GRAPH_STYLE_MAX;
	int color = GP_WIDGETS_COL_TEXT;
	enum gp_widget_graph_flags flags = 0;
	char *x_label = NULL, *y_label = NULL;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case COLOR:
			color = gp_widgets_color_name_idx(val->val_str);
			if (color < 0) {
				gp_json_warn(json, "Invalid color '%s'", val->val_str);
				color = GP_WIDGETS_COL_TEXT;
			}
		break;
		case MAX_DATA_POINTS:
			if (val->val_int <= 0)
				gp_json_warn(json, "data_points must be > 0!");
			else
				data_points = val->val_int;
		break;
		case MIN_H:
			switch (val->type) {
			case GP_JSON_INT:
				if (val->val_int < 0)
					gp_json_warn(json, "Size must be >= 0!");
				else
					h.px = val->val_int;
			break;
			case GP_JSON_STR:
				if (gp_widget_size_units_parse(val->val_str, &h))
					gp_json_warn(json, "Invalid size!");
			break;
			default:
				gp_json_warn(json, "Invalid size type!");
			}
		break;
		case MIN_W:
			switch (val->type) {
			case GP_JSON_INT:
				if (val->val_int < 0)
					gp_json_warn(json, "Size must be >= 0!");
				else
					w.px = val->val_int;
			break;
			case GP_JSON_STR:
				if (gp_widget_size_units_parse(val->val_str, &w))
					gp_json_warn(json, "Invalid size!");
			break;
			default:
				gp_json_warn(json, "Invalid size type!");
			}
		break;
		case STYLE:
			style = str_to_style(val->val_str);
			if (style == GP_WIDGET_GRAPH_STYLE_MAX)
				gp_json_warn(json, "Invalid graph style");
		break;
		case X_LABEL:
			free(x_label);
			x_label = strdup(val->val_str);
		break;
		case Y_LABEL:
			free(y_label);
			y_label = strdup(val->val_str);
		break;
		case ERROR_BARS:
			if (val->val_bool)
				flags |= GP_WIDGET_GRAPH_ERROR_BARS;
		break;
		}
	}

	gp_widget *ret = gp_widget_graph_new(w, h, x_label, y_label, data_points);

	free(x_label);
	free(y_label);

	if (!ret)
		return NULL;

	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(ret);

	if (style != GP_WIDGET_GRAPH_STYLE_MAX)
		graph->graph_style = style;

	graph->color = color;
	graph->flags = flags;

	return ret;
}

static void free_(gp_widget *self)
{
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	free(graph->x_label);
	free(graph->y_label);
	free(graph->data);
}

struct gp_widget_ops gp_widget_graph_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.free = free_,
	.from_json = json_to_graph,
	.id = "graph",
};

gp_widget *gp_widget_graph_new(gp_widget_size min_w, gp_widget_size min_h,
                               const char *x_label, const char *y_label,
                               size_t max_data_points)
{
	gp_widget *ret;

	ret = gp_widget_new(GP_WIDGET_GRAPH, GP_WIDGET_CLASS_NONE, sizeof(struct gp_widget_graph));
	if (!ret)
		return NULL;

	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(ret);

	graph->data = malloc(sizeof(struct gp_widget_graph_point) * max_data_points);
	if (!graph->data) {
		free(ret);
		return NULL;
	}

	if (x_label && *x_label)
		graph->x_label = strdup(x_label);

	if (y_label && *y_label)
		graph->y_label = strdup(y_label);

	graph->min_w = min_w;
	graph->min_h = min_h;

	gp_cbuffer_init(&graph->data_idx, max_data_points);

	return ret;
}

static void new_min_max(struct gp_widget_graph *graph)
{
	gp_cbuffer_iter iter;

	if (!gp_cbuffer_used(&graph->data_idx)) {
		graph->min_x = 0;
		graph->max_x = 0;

		if (!graph->min_y_fixed)
			graph->min_y = 0;

		if (!graph->max_y_fixed)
			graph->max_y = 0;

		return;
	}

	size_t first = gp_cbuffer_first(&graph->data_idx);

	graph->min_x = graph->data[first].x;
	graph->max_x = graph->data[first].x;

	if (!graph->min_y_fixed)
		graph->min_y = graph->data[first].y;

	if (!graph->max_y_fixed)
		graph->max_y = graph->data[first].y;

	GP_CBUFFER_FOREACH(&graph->data_idx, &iter) {
		double x = graph->data[iter.idx].x;
		double y = graph->data[iter.idx].y;

		graph->min_x = GP_MIN(graph->min_x, x);
		graph->max_x = GP_MAX(graph->max_x, x);
		if (!graph->min_y_fixed)
			graph->min_y = GP_MIN(graph->min_y, y);
		if (!graph->max_y_fixed)
			graph->max_y = GP_MAX(graph->max_y, y);
	}

	if (graph->graph_style != GP_WIDGET_GRAPH_BAR &&
	    graph->graph_style != GP_WIDGET_GRAPH_STEM)
		return;

	/* Bars start at zero and the error bars have to fit as well */
	if (!graph->min_y_fixed)
		graph->min_y = GP_MIN(graph->min_y, 0);

	if (!graph->max_y_fixed)
		graph->max_y = GP_MAX(graph->max_y, 0);

	if (!(graph->flags & GP_WIDGET_GRAPH_ERROR_BARS))
		return;

	GP_CBUFFER_FOREACH(&graph->data_idx, &iter) {
		double err = fabs(graph->data[iter.idx].err);
		double y = graph->data[iter.idx].y;

		if (!graph->min_y_fixed)
			graph->min_y = GP_MIN(graph->min_y, y - err);
		if (!graph->max_y_fixed)
			graph->max_y = GP_MAX(graph->max_y, y + err);
	}
}

void gp_widget_graph_style_set(gp_widget *self, enum gp_widget_graph_style style)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	if (style >= GP_WIDGET_GRAPH_STYLE_MAX) {
		GP_WARN("Invalid graph style %i\n", style);
		return;
	}

	if (graph->graph_style == style)
		return;

	graph->graph_style = style;

	new_min_max(graph);
	gp_widget_redraw(self);
}

enum gp_widget_graph_style gp_widget_graph_style_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, GP_WIDGET_GRAPH_STYLE_MAX);
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	return graph->graph_style;
}

void gp_widget_graph_points_clear(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	gp_cbuffer_reset(&graph->data_idx);

	new_min_max(graph);
	gp_widget_redraw(self);
}

void gp_widget_graph_point_add(gp_widget *self, double x, double y)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	size_t pos = gp_cbuffer_append(&graph->data_idx);

	graph->data[pos].x = x;
	graph->data[pos].y = y;

	new_min_max(graph);

	gp_widget_redraw(self);
}

static void points_changed(gp_widget *self)
{
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	new_min_max(graph);

	gp_widget_redraw(self);
}

void gp_widget_graph_points_set(gp_widget *self,
                                struct gp_widget_graph_point *data,
				size_t max_data_points,
                                size_t data_points)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	if (!data || !max_data_points) {
		GP_WARN("Invalid data=%p or max_data_points=%zu", data, max_data_points);
		return;
	}

	free(graph->data);
	graph->data = data;

	gp_cbuffer_init2(&graph->data_idx, max_data_points, data_points);

	points_changed(self);
}

void gp_widget_graph_points_copy(gp_widget *self,
                                 const struct gp_widget_graph_point *data,
                                 size_t data_points)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	if (data_points != gp_cbuffer_size(&graph->data_idx)) {
		struct gp_widget_graph_point *tmp;

		tmp = realloc(graph->data, sizeof(struct gp_widget_graph_point) * data_points);
		if (!tmp) {
			GP_WARN("Malloc failed :-(");
			return;
		}

		graph->data = tmp;
	}

	memcpy(graph->data, data, sizeof(struct gp_widget_graph_point) * data_points);
	gp_cbuffer_init2(&graph->data_idx, data_points, data_points);

	points_changed(self);
}

void gp_widget_graph_yrange_set(gp_widget *self, double min_y, double max_y)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	graph->min_y = min_y;
	graph->max_y = max_y;

	graph->min_y_fixed = 1;
	graph->max_y_fixed = 1;
}

void gp_widget_graph_ymin_set(gp_widget *self, double min_y)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	graph->min_y = min_y;

	graph->min_y_fixed = 1;

	new_min_max(graph);
}

void gp_widget_graph_ymax_set(gp_widget *self, double max_y)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	graph->max_y = max_y;

	graph->max_y_fixed = 1;

	new_min_max(graph);
}

void gp_widget_graph_yrange_clear(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	graph->min_y_fixed = 0;
	graph->max_y_fixed = 0;

	new_min_max(graph);
}

void gp_widget_graph_flags_set(gp_widget *self, enum gp_widget_graph_flags flags)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	if (graph->flags == flags)
		return;

	graph->flags = flags;

	new_min_max(graph);
	gp_widget_redraw(self);
}

enum gp_widget_graph_flags gp_widget_graph_flags_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, 0);
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	return graph->flags;
}

static void label_set(gp_widget *self, char **label, const char *new_label)
{
	char *tmp = NULL;

	if (new_label && *new_label) {
		tmp = strdup(new_label);
		if (!tmp) {
			GP_WARN("Malloc failed :-(");
			return;
		}
	}

	free(*label);
	*label = tmp;

	gp_widget_resize(self);
	gp_widget_redraw(self);
}

void gp_widget_graph_x_label_set(gp_widget *self, const char *x_label)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	label_set(self, &graph->x_label, x_label);
}

const char *gp_widget_graph_x_label_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, NULL);
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	return graph->x_label;
}

void gp_widget_graph_y_label_set(gp_widget *self, const char *y_label)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	label_set(self, &graph->y_label, y_label);
}

const char *gp_widget_graph_y_label_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, NULL);
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	return graph->y_label;
}

void gp_widget_graph_color_set(gp_widget *self, enum gp_widgets_color color)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	graph->color = color;
}
