//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_json.h>

struct gp_widget_graph {
	gp_widget_size min_w;
	gp_widget_size min_h;

	const char *x_label;
	const char *y_label;

	long min_y_fixed:1;
	long max_y_fixed:1;

	/* Graph scaling window */
	double min_x;
	double max_x;
	double min_y;
	double max_y;

	/* Graph data */
	enum gp_widget_graph_style graph_style;
	enum gp_widgets_color color;
	gp_cbuffer data_idx;
	struct gp_widget_graph_point *data;
};

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	return gp_widget_size_units_get(&graph->min_w, ctx);
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	return gp_widget_size_units_get(&graph->min_h, ctx);
}

static gp_coord transform_x(struct gp_widget_graph *graph, double x, gp_coord w)
{
	double range = graph->max_x - graph->min_x;

	return round(w * fabs(x - graph->min_x) / range);
}

static gp_coord transform_y(struct gp_widget_graph *graph, double y, gp_coord h)
{
	double range = graph->max_y - graph->min_y;

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

	gp_sub_pixmap(ctx->buf, &pix, x, y, w, h);

	switch (graph->graph_style) {
	case GP_WIDGET_GRAPH_POINT:
		render_point_graph(graph, &pix, w, h, ctx);
	break;
	case GP_WIDGET_GRAPH_LINE:
		render_line_graph(graph, &pix, w, h, ctx);
	break;
	case GP_WIDGET_GRAPH_FILL:
		render_fill_graph(graph, &pix, w, h, ctx);
	break;
	default:
		GP_WARN("Invalid graph type %i", graph->graph_style);
	}
}

const char *gp_widget_graph_style_names[GP_WIDGET_GRAPH_STYLE_MAX] = {
	"point",
	"line",
	"fill",
};

enum keys {
	COLOR,
	MAX_DATA_POINTS,
	MIN_H,
	MIN_W,
	STYLE,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(COLOR, "color", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(MAX_DATA_POINTS, "max_data_points", GP_JSON_INT),
	GP_JSON_OBJ_ATTR_IDX(MIN_H, "min_h", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR_IDX(MIN_W, "min_w", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR_IDX(STYLE, "style", GP_JSON_STR),
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
		}
	}

	gp_widget *ret = gp_widget_graph_new(w, h, NULL, NULL, data_points);

	if (!ret)
		return NULL;

	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(ret);

	if (style != GP_WIDGET_GRAPH_STYLE_MAX)
		graph->graph_style = style;

	graph->color = color;

	return ret;
}

struct gp_widget_ops gp_widget_graph_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
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

	if (x_label)
		graph->x_label = strdup(x_label);

	if (y_label)
		graph->y_label = strdup(y_label);

	graph->min_w = min_w;
	graph->min_h = min_h;

	gp_cbuffer_init(&graph->data_idx, max_data_points);

	return ret;
}

static void new_min_max(struct gp_widget_graph *graph)
{
	gp_cbuffer_iter iter;

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
}

void gp_widget_graph_ymax_set(gp_widget *self, double max_y)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	graph->max_y = max_y;

	graph->max_y_fixed = 1;
}

void gp_widget_graph_yrange_clear(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	graph->min_y_fixed = 0;
	graph->max_y_fixed = 0;

	new_min_max(graph);
}

void gp_widget_graph_color_set(gp_widget *self, enum gp_widgets_color color)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = GP_WIDGET_PAYLOAD(self);

	graph->color = color;
}
