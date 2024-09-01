//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_graph.h
 * @brief An XY graph.
 *
 * Graph widget JSON attributes
 * ----------------------------
 *
 * |      Attribute      |  Type  | Default | Description                                               |
 * |---------------------|--------|---------|-----------------------------------------------------------|
 * |     **color**       | string | "text"  | A graph color, parsed by gp_widgets_color_name_idx().     |
 * | **max_data_points** |  uint  |   100   | Maximal number of graph data points.                      |
 * |     **min_h**       | string |         | Minimal height, parsed by gp_widget_size_units_parse().   |
 * |     **min_w**       | string |         | Minimal width, parsed by gp_widget_size_units_parse().    |
 * |     **style**       | string |         | One of "point", "line" or "fill", #gp_widget_graph_style. |
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
	/**
	 * @brief Point graph.
	 *
	 * Graph is drawn using individual points.
	 */
	GP_WIDGET_GRAPH_POINT,
	/**
	 * @brief Points connected by line.
	 *
	 * The graph points are connected by a line.
	 */
	GP_WIDGET_GRAPH_LINE,
	/**
	 * @brief Area below graph is filled.
	 *
	 * The area bellow the line created by the graph points is filled.
	 */
	GP_WIDGET_GRAPH_FILL,
	/** @brief Maximal number of styles */
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
	/** @brief An x value. */
	double x;
	/** @brief An y value. */
	double y;
};

/**
 * @brief Allocates and initializes a new graph widget.
 *
 * @param min_w A minimal width.
 * @param min_h A minimal height.
 * @param x_label A label for the x axis.
 * @param y_label A label for the y axis.
 * @param max_data_points A maximal number of graph data points.
 *
 * @return A graph widget.
 */
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
