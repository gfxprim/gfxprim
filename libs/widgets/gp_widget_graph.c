//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_json.h>

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	return gp_widget_size_units_get(&self->graph->min_w, ctx);
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	return gp_widget_size_units_get(&self->graph->min_h, ctx);
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
	size_t i;

	gp_size line_th = (ctx->fr_thick+1)/2;
	gp_size circle_r = line_th;
	gp_pixel col = ctx->colors[graph->color];

	gp_coord px = transform_x(graph, graph->data[graph->data_first].x, w-2*circle_r-1) + circle_r;
	gp_coord py = transform_y(graph, graph->data[graph->data_first].y, h-2*circle_r-1) + circle_r;

	for (i = (graph->data_first + 1)%graph->data_points; i != graph->data_last; i = (i+1) % graph->data_points) {
		gp_coord dx = transform_x(graph, graph->data[i].x, w-2*circle_r-1) + circle_r;
		gp_coord dy = transform_y(graph, graph->data[i].y, h-2*circle_r-1) + circle_r;

		gp_line_th(pix, px, py, dx, dy, line_th, col);

		px = dx; py = dy;
	}

	for (i = graph->data_first; i != graph->data_last; i = (i+1) % graph->data_points) {
		gp_coord dx = transform_x(graph, graph->data[i].x, w-2*circle_r-1) + circle_r;
		gp_coord dy = transform_y(graph, graph->data[i].y, h-2*circle_r-1) + circle_r;

		gp_fill_circle(pix, dx, dy, circle_r, col);
	}
}


static void render_point_graph(struct gp_widget_graph *graph,
                               gp_pixmap *pix, gp_coord w, gp_coord h,
                               const gp_widget_render_ctx *ctx)
{
	size_t i;

	gp_size line_th = (ctx->fr_thick+1)/2;
	gp_size circle_r = line_th*2;
	gp_pixel col = ctx->colors[graph->color];

	for (i = graph->data_first; i != graph->data_last; i = (i+1) % graph->data_points) {
		gp_coord dx = transform_x(graph, graph->data[i].x, w-2*circle_r-1) + circle_r;
		gp_coord dy = transform_y(graph, graph->data[i].y, h-2*circle_r-1) + circle_r;

		gp_fill_circle(pix, dx, dy, circle_r, col);
	}
}

static void render_fill_graph(struct gp_widget_graph *graph,
                              gp_pixmap *pix, gp_size w, gp_size h,
                              const gp_widget_render_ctx *ctx)
{
	gp_coord poly[graph->data_points * 2 + 4];
	gp_pixel col = ctx->colors[graph->color];
	size_t i = 0, pos = 1;

	poly[0] = 0;
	poly[1] = h - 1;

	for (i = graph->data_first; i != graph->data_last; i = (i+1) % graph->data_points) {
		poly[2 * pos] = transform_x(graph, graph->data[i].x, w - 1);
		poly[2 * pos + 1] = transform_y(graph, graph->data[i].y, h - 1);
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
	struct gp_widget_graph *graph = self->graph;
	gp_coord x = self->x + offset->x;
	gp_coord y = self->y + offset->y;
	gp_size w = self->w;
	gp_size h = self->h;
	gp_pixmap pix;

	(void) flags;

	gp_fill_rect_xywh(ctx->buf, x, y, w, h, ctx->fg_color);
        gp_widget_ops_blit(ctx, x, y, w, h);

	gp_sub_pixmap(ctx->buf, &pix, x, y, w, h);

	switch (graph->graph_type) {
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
		GP_WARN("Invalid graph type %i", graph->graph_type);
	}
}

enum keys {
	H,
	W,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("h", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR("w", GP_JSON_VOID),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_graph(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	gp_widget_size w = {};
	gp_widget_size h = {};

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case H:
			switch (val->type) {
			case GP_JSON_INT:
				if (val->val_int < 0)
					gp_json_warn(json, "Size must be > 0!");
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
		case W:
			switch (val->type) {
			case GP_JSON_INT:
				if (val->val_int < 0)
					gp_json_warn(json, "Size must be > 0!");
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
		}
	}

	return gp_widget_pixmap_new(w, h, NULL, NULL);
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

	ret->graph->data = malloc(sizeof(struct gp_widget_graph_point) * max_data_points);
	if (!ret) {
		free(ret);
		return NULL;
	}

	if (x_label)
		ret->graph->x_label = strdup(x_label);

	if (y_label)
		ret->graph->y_label = strdup(y_label);

	ret->graph->min_w = min_w;
	ret->graph->min_h = min_h;
	ret->graph->data_first = 0;
	ret->graph->data_last = 0;
	ret->graph->data_points = max_data_points;

	return ret;
}

static void new_min_max(struct gp_widget_graph *graph)
{
	size_t i;

	graph->min_x = graph->data[graph->data_first].x;
	graph->max_x = graph->data[graph->data_first].x;

	if (!graph->min_y_fixed)
		graph->min_y = graph->data[graph->data_first].y;

	if (!graph->max_y_fixed)
		graph->max_y = graph->data[graph->data_first].y;

	for (i = graph->data_first; i != graph->data_last; i = (i + 1) % graph->data_points) {
		double x = graph->data[i].x;
		double y = graph->data[i].y;

		graph->min_x = GP_MIN(graph->min_x, x);
		graph->max_x = GP_MAX(graph->max_x, x);
		if (!graph->min_y_fixed)
			graph->min_y = GP_MIN(graph->min_y, y);
		if (!graph->max_y_fixed)
			graph->max_y = GP_MAX(graph->max_y, y);
	}
}

const char *gp_widget_graph_type_names[GP_WIDGET_GRAPH_TYPE_MAX] = {
	"point",
	"line",
	"fill",
};

void gp_widget_graph_type_set(gp_widget *self, enum gp_widget_graph_type type)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = self->graph;

	if (type >= GP_WIDGET_GRAPH_TYPE_MAX) {
		GP_WARN("Invalid graph type %i\n", type);
		return;
	}

	if (graph->graph_type == type)
		return;

	graph->graph_type = type;

	gp_widget_redraw(self);
}

void gp_widget_graph_point_add(gp_widget *self, double x, double y)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_GRAPH, );
	struct gp_widget_graph *graph = self->graph;

	size_t pos = graph->data_last;

	graph->data[pos].x = x;
	graph->data[pos].y = y;

	graph->data_last = (graph->data_last + 1) % graph->data_points;

	if (graph->data_last == graph->data_first)
		graph->data_first = (graph->data_first + 1) % graph->data_points;

	new_min_max(graph);

	gp_widget_redraw(self);
}
