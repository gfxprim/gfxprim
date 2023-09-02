//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_GRAPH_H
#define GP_WIDGET_GRAPH_H

#include <widgets/gp_widget_types.h>
#include <widgets/gp_widget_size_units.h>

enum gp_widget_graph_type {
	GP_WIDGET_GRAPH_POINT,
	GP_WIDGET_GRAPH_LINE,
	GP_WIDGET_GRAPH_FILL,
	GP_WIDGET_GRAPH_TYPE_MAX,
};

extern const char *gp_widget_graph_type_names[GP_WIDGET_GRAPH_TYPE_MAX];

struct gp_widget_graph_point {
	double x, y;
};

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
	enum gp_widget_graph_type graph_type;
	enum gp_widgets_color color;
	size_t data_points;
	size_t data_first;
	size_t data_last;
	struct gp_widget_graph_point *data;
};

gp_widget *gp_widget_graph_new(gp_widget_size min_w, gp_widget_size min_h,
                               const char *x_label, const char *y_label,
                               size_t max_data_points);

void gp_widget_graph_point_add(gp_widget *self, double x, double y);

void gp_widget_graph_type_set(gp_widget *self, enum gp_widget_graph_type type);

#endif /* GP_WIDGET_GRAPH_H */
