//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_graph.h
 * @brief An XY graph.
 */

#ifndef GP_WIDGET_GRAPH_H
#define GP_WIDGET_GRAPH_H

#include <utils/gp_cbuffer.h>
#include <widgets/gp_widget_types.h>
#include <widgets/gp_widget_size_units.h>

/**
 * @brief A graph drawing style.
 */
enum gp_widget_graph_style {
	/** Point graph */
	GP_WIDGET_GRAPH_POINT,
	/** Points connected by line */
	GP_WIDGET_GRAPH_LINE,
	/** Area below graph is filled */
	GP_WIDGET_GRAPH_FILL,
	/** Maximal number of styles */
	GP_WIDGET_GRAPH_STYLE_MAX,
};

/**
 * @brief A graph style names.
 */
extern const char *gp_widget_graph_style_names[GP_WIDGET_GRAPH_STYLE_MAX];

/**
 * @brief A graph point.
 */
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

/**
 * @brief Adds a graph point.
 *
 * The graph data are stored in a circular buffer, when buffer is full the last
 * data point is discarded before new point is added.
 *
 * @param self A graph widget.
 * @param x A x coordinate of the data point.
 * @param y A y coordinate of the data point.
 */
void gp_widget_graph_point_add(gp_widget *self, double x, double y);

/**
 * @brief Sets a graph style.
 *
 * @param self A graph widget.
 * @param style A graph style.
 */
void gp_widget_graph_style_set(gp_widget *self, enum gp_widget_graph_style style);

/**
 * @brief Sets graph y range.
 *
 * If range is not set the graph is autorange mode and the data will fit the y scale.
 *
 * @param self A graph widget.
 * @param min_y New y minimum.
 * @param max_y New y maximum.
 */
void gp_widget_graph_yrange_set(gp_widget *self, double min_y, double max_y);

/**
 * @brief Sets graph y minimum.
 *
 * If range is not set the graph is autorange mode and the data will fit the y scale.
 *
 * @param self A graph widget.
 * @param min_y New y minimum.
 */
void gp_widget_graph_ymin_set(gp_widget *self, double min_y);

/**
 * @brief Sets graph y maximum.
 *
 * If range is not set the graph is autorange mode and the data will fit the y scale.
 *
 * @param self A graph widget.
 * @param max_y New y maximum.
 */
void gp_widget_graph_ymax_set(gp_widget *self, double max_y);

/**
 * @brief Clears graph y range.
 *
 * Returns graph widget into autorange mode.
 *
 * @param self A graph widget.
 */
void gp_widget_graph_yrange_clear(gp_widget *self);

/**
 * @brief Sets graph color.
 *
 * @param self A graph widget.
 * @param color An color index.
 */
void gp_widget_graph_color_set(gp_widget *self, enum gp_widgets_color color);

#endif /* GP_WIDGET_GRAPH_H */
