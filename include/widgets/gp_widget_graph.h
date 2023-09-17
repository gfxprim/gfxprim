//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_GRAPH_H
#define GP_WIDGET_GRAPH_H

#include <utils/gp_cbuffer.h>
#include <widgets/gp_widget_types.h>
#include <widgets/gp_widget_size_units.h>

enum gp_widget_graph_style {
	GP_WIDGET_GRAPH_POINT,
	GP_WIDGET_GRAPH_LINE,
	GP_WIDGET_GRAPH_FILL,
	GP_WIDGET_GRAPH_STYLE_MAX,
};

extern const char *gp_widget_graph_style_names[GP_WIDGET_GRAPH_STYLE_MAX];

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
	enum gp_widget_graph_style graph_style;
	enum gp_widgets_color color;
	gp_cbuffer data_idx;
	struct gp_widget_graph_point *data;
};

gp_widget *gp_widget_graph_new(gp_widget_size min_w, gp_widget_size min_h,
                               const char *x_label, const char *y_label,
                               size_t max_data_points);

void gp_widget_graph_point_add(gp_widget *self, double x, double y);

void gp_widget_graph_style_set(gp_widget *self, enum gp_widget_graph_style type);

/**
 * @brief Sets graph y range.
 *
 * If range is not set the graph is autorange mode and the data will fit the y scale.
 *
 * @self A graph widget.
 * @min_y New y minimum.
 * @max_y New y maximum.
 */
void gp_widget_graph_yrange_set(gp_widget *self, double min_y, double max_y);

/**
 * @brief Sets graph y minimum.
 *
 * If range is not set the graph is autorange mode and the data will fit the y scale.
 *
 * @self A graph widget.
 * @min_y New y minimum.
 */
void gp_widget_graph_ymin_set(gp_widget *self, double min_y);

/**
 * @brief Sets graph y maximum.
 *
 * If range is not set the graph is autorange mode and the data will fit the y scale.
 *
 * @self A graph widget.
 * @max_y New y maximum.
 */
void gp_widget_graph_ymax_set(gp_widget *self, double max_y);

/**
 * @brief Clears graph y range.
 *
 * Returns graph widget into autorange mode.
 *
 * @self A graph widget.
 */
void gp_widget_graph_yrange_clear(gp_widget *self);

/**
 * @brief Sets graph color.
 *
 * @self A graph widget.
 * @color An color index.
 */
void gp_widget_graph_color_set(gp_widget *self, enum gp_widgets_color color);

#endif /* GP_WIDGET_GRAPH_H */
