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
 * |      Attribute      |  Type  | Default | Description                                                      |
 * |---------------------|--------|---------|------------------------------------------------------------------|
 * |     **color**       | string | "text"  | A graph color, parsed by gp_widgets_color_name_idx().            |
 * |   **error_bars**    |  bool  |  false  | Sets the #GP_WIDGET_GRAPH_ERROR_BARS flag.                       |
 * | **max_data_points** |  uint  |   100   | Maximal number of graph data points.                             |
 * |     **min_h**       | string |         | Minimal height, parsed by gp_widget_size_units_parse().          |
 * |     **min_w**       | string |         | Minimal width, parsed by gp_widget_size_units_parse().           |
 * |     **style**       | string |         | One of "point", "line", "fill" or "bar", #gp_widget_graph_style. |
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
	/**
	 * @brief Bar graph.
	 *
	 * Each point is drawn as a bar that starts at zero, or at the
	 * nearest edge if zero is outside of the y range. The bars are drawn
	 * evenly spaced in the order the points were added and the x
	 * coordinate is ignored, unless #GP_WIDGET_GRAPH_ERROR_BARS is set. The y
	 * range in the autorange mode always includes zero.
	 */
	GP_WIDGET_GRAPH_BAR,
	/** @brief Maximal number of styles, also returned on error. */
	GP_WIDGET_GRAPH_STYLE_MAX,
};

/**
 * @brief A graph style names.
 */
extern const char *gp_widget_graph_style_names[GP_WIDGET_GRAPH_STYLE_MAX];

/**
 * @brief Graph flags.
 */
enum gp_widget_graph_flags {
	/**
	 * @brief Reinterpret the x coordinate as an error.
	 *
	 * Applies only to the #GP_WIDGET_GRAPH_BAR style, the x coordinate,
	 * accessible as gp_widget_graph_point::err as well, is drawn as an
	 * error bar from y - |err| to y + |err| on top of each bar, e.g. a
	 * standard deviation. Zero error draws no error bar. The y range in
	 * the autorange mode includes the error bars.
	 */
	GP_WIDGET_GRAPH_ERROR_BARS = 0x01,
};

/**
 * @brief A graph point.
 */
struct gp_widget_graph_point {
	union {
		/** @brief An x value. */
		double x;
		/**
		 * @brief An error value.
		 *
		 * Used instead of the x value if #GP_WIDGET_GRAPH_ERROR_BARS is
		 * set, see #gp_widget_graph_flags.
		 */
		double err;
	};
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
 * @brief Replaces graph data with an array, the widget takes the ownership.
 *
 * The array replaces the graph data buffer, its size becomes the new maximal
 * number of graph data points. The array is freed by the widget, either when
 * it's replaced or when the widget is freed.
 *
 * @param self A graph widget.
 * @param data An array allocated by malloc().
 * @param max_data_points The size of the data array.
 * @param data_points A number of points in the array, must be >= 1.
 */
void gp_widget_graph_points_set(gp_widget *self,
                                struct gp_widget_graph_point *data,
                                size_t max_data_points,
                                size_t data_points);


/**
 * @brief Clears all points in graph.
 *
 * Resets the graph to the intial state. That means that after this call there
 * are no points in graph, but the buffer is still allocated with the correct
 * size.
 *
 * @param self A graph widget.
 */
void gp_widget_graph_points_clear(gp_widget *self);

/**
 * @brief Replaces graph data with a copy of an array.
 *
 * The graph data buffer is resized if the array does not fit. The graph is
 * left untouched if the buffer cannot be resized.
 *
 * @param self A graph widget.
 * @param data An array of points, the caller keeps the ownership.
 * @param data_points A number of points in the array, must be >= 1.
 */
void gp_widget_graph_points_copy(gp_widget *self,
                                 const struct gp_widget_graph_point *data,
                                 size_t data_points);

/**
 * @brief Sets graph style.
 *
 * @param self A graph widget.
 * @param style A graph style.
 */
void gp_widget_graph_style_set(gp_widget *self, enum gp_widget_graph_style style);

/**
 * @brief Returns graph style.
 *
 * @param self A graph widget.
 */
enum gp_widget_graph_style gp_widget_graph_style_get(gp_widget *self);

/**
 * @brief Sets graph flags.
 *
 * @param self A graph widget.
 * @param flags A bitwise or of #gp_widget_graph_flags.
 */
void gp_widget_graph_flags_set(gp_widget *self, enum gp_widget_graph_flags flags);

/**
 * @brief Returns graph flags.
 *
 * @param self A graph widget.
 *
 * @return A bitwise or of #gp_widget_graph_flags.
 */
enum gp_widget_graph_flags gp_widget_graph_flags_get(gp_widget *self);

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
