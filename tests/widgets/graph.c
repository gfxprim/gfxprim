// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"

#define MAX_POINTS 4

static gp_widget *graph_new(void)
{
	return gp_widget_graph_new(GP_WIDGET_SIZE(10, 0, 0),
	                           GP_WIDGET_SIZE(10, 0, 0),
	                           "x", "y", MAX_POINTS);
}

static struct gp_widget_graph_point *points_alloc(size_t cnt)
{
	struct gp_widget_graph_point *ret = malloc(sizeof(*ret) * cnt);
	size_t i;

	if (!ret)
		return NULL;

	for (i = 0; i < cnt; i++) {
		ret[i].x = i;
		ret[i].y = 2 * i;
	}

	return ret;
}

static int graph_new_free(void)
{
	gp_widget *graph = graph_new();

	if (!graph) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_free(graph);

	return TST_PASSED;
}

static int points_set(size_t cnt)
{
	struct gp_widget_graph_point *points = points_alloc(cnt);
	gp_widget *graph = graph_new();
	size_t i;

	if (!points || !graph) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_graph_points_set(graph, points, cnt, cnt);

	/* The buffer has to be usable and wrap around after the set */
	for (i = 0; i < 2 * cnt + 1; i++)
		gp_widget_graph_point_add(graph, i, i);

	gp_widget_free(graph);

	return TST_PASSED;
}

static int graph_points_set_fewer(void)
{
	return points_set(MAX_POINTS - 2);
}

static int graph_points_set_more(void)
{
	return points_set(2 * MAX_POINTS);
}

static int graph_points_set_replace(void)
{
	gp_widget *graph = graph_new();
	struct gp_widget_graph_point *p1 = points_alloc(3);
	struct gp_widget_graph_point *p2 = points_alloc(5);

	if (!graph || !p1 || !p2) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_graph_points_set(graph, p1, 3, 3);
	gp_widget_graph_points_set(graph, p2, 5, 3);

	gp_widget_free(graph);

	return TST_PASSED;
}

static int graph_points_set_zero(void)
{
	gp_widget *graph = graph_new();
	struct gp_widget_graph_point *points = points_alloc(1);

	if (!graph || !points) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_graph_points_set(graph, points, 1, 0);
	gp_widget_graph_points_set(graph, NULL, 0, 0);
	gp_widget_graph_point_add(graph, 1, 1);

	gp_widget_free(graph);

	return TST_PASSED;
}

static int points_copy(size_t cnt)
{
	struct gp_widget_graph_point *points = points_alloc(cnt);
	gp_widget *graph = graph_new();
	size_t i;

	if (!points || !graph) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_graph_points_copy(graph, points, cnt);

	/* The caller keeps the array, the graph must not touch it anymore */
	free(points);

	for (i = 0; i < 2 * GP_MAX(cnt, (size_t)MAX_POINTS) + 1; i++)
		gp_widget_graph_point_add(graph, i, i);

	gp_widget_free(graph);

	return TST_PASSED;
}

static int graph_points_copy_fewer(void)
{
	return points_copy(MAX_POINTS - 2);
}

static int graph_points_copy_exact(void)
{
	return points_copy(MAX_POINTS);
}

static int graph_points_copy_grow(void)
{
	return points_copy(3 * MAX_POINTS);
}

static int graph_points_copy_zero(void)
{
	gp_widget *graph = graph_new();

	if (!graph) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_graph_point_add(graph, 1, 1);
	gp_widget_graph_points_copy(graph, NULL, 0);
	gp_widget_graph_point_add(graph, 1, 1);

	gp_widget_free(graph);

	return TST_PASSED;
}

static int graph_flags(void)
{
	gp_widget *graph = graph_new();
	int ret = TST_PASSED;

	if (!graph) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	if (gp_widget_graph_flags_get(graph)) {
		tst_msg("Flags not zero by default");
		ret = TST_FAILED;
	}

	gp_widget_graph_flags_set(graph, GP_WIDGET_GRAPH_ERROR_BARS);

	if (gp_widget_graph_flags_get(graph) != GP_WIDGET_GRAPH_ERROR_BARS) {
		tst_msg("Failed to set GP_WIDGET_GRAPH_ERROR_BARS");
		ret = TST_FAILED;
	}

	gp_widget_graph_flags_set(graph, 0);

	if (gp_widget_graph_flags_get(graph)) {
		tst_msg("Failed to clear flags");
		ret = TST_FAILED;
	}

	gp_widget_free(graph);

	return ret;
}

static int graph_style_names(void)
{
	static const char *const names[] = {
		[GP_WIDGET_GRAPH_POINT] = "point",
		[GP_WIDGET_GRAPH_LINE] = "line",
		[GP_WIDGET_GRAPH_FILL] = "fill",
		[GP_WIDGET_GRAPH_BAR] = "bar",
		[GP_WIDGET_GRAPH_STEM] = "stem",
		[GP_WIDGET_GRAPH_STEP] = "step",
	};
	int i, ret = TST_PASSED;

	if (GP_ARRAY_SIZE(names) != GP_WIDGET_GRAPH_STYLE_MAX) {
		tst_msg("Style added without updating the test");
		return TST_FAILED;
	}

	for (i = 0; i < GP_WIDGET_GRAPH_STYLE_MAX; i++) {
		if (!gp_widget_graph_style_names[i] ||
		    strcmp(gp_widget_graph_style_names[i], names[i])) {
			tst_msg("Wrong style %i name '%s' expected '%s'", i,
			        gp_widget_graph_style_names[i], names[i]);
			ret = TST_FAILED;
		}
	}

	return ret;
}

static int label_check(const char *what, const char *label, const char *exp)
{
	if (!label && !exp)
		return 0;

	if (!label || !exp || strcmp(label, exp)) {
		tst_msg("Wrong %s '%s' expected '%s'", what,
		        label ? label : "NULL", exp ? exp : "NULL");
		return 1;
	}

	return 0;
}

static int graph_labels(void)
{
	gp_widget *graph = graph_new();
	int fail = 0;

	if (!graph) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	fail |= label_check("x label", gp_widget_graph_x_label_get(graph), "x");
	fail |= label_check("y label", gp_widget_graph_y_label_get(graph), "y");

	gp_widget_graph_x_label_set(graph, "Days");
	gp_widget_graph_y_label_set(graph, "Passes");

	fail |= label_check("x label", gp_widget_graph_x_label_get(graph), "Days");
	fail |= label_check("y label", gp_widget_graph_y_label_get(graph), "Passes");

	gp_widget_graph_x_label_set(graph, NULL);
	gp_widget_graph_y_label_set(graph, "");

	fail |= label_check("x label", gp_widget_graph_x_label_get(graph), NULL);
	fail |= label_check("y label", gp_widget_graph_y_label_get(graph), NULL);

	gp_widget_graph_x_label_set(graph, "Weeks");

	fail |= label_check("x label", gp_widget_graph_x_label_get(graph), "Weeks");

	gp_widget_free(graph);

	return fail ? TST_FAILED : TST_PASSED;
}

struct json_tcase {
	const char *json;
	enum gp_widget_graph_flags flags;
	enum gp_widget_graph_style style;
	const char *x_label;
	const char *y_label;
};

static int graph_json(struct json_tcase *t)
{
	gp_widget *graph;
	int ret = TST_PASSED;

	graph = gp_widget_from_json_str(t->json, NULL, NULL);
	if (!graph) {
		tst_msg("Failed to load JSON");
		return TST_FAILED;
	}

	if (graph->type != GP_WIDGET_GRAPH) {
		tst_msg("Wrong widget type!");
		ret = TST_FAILED;
		goto exit;
	}

	if (gp_widget_graph_style_get(graph) != t->style) {
		tst_msg("Invalid graph type %i expected %i",
			gp_widget_graph_style_get(graph), t->style);
		ret = TST_FAILED;
	}

	if (gp_widget_graph_flags_get(graph) != t->flags) {
		tst_msg("Wrong flags %x expected %x",
		        gp_widget_graph_flags_get(graph), t->flags);
		ret = TST_FAILED;
	}

	if (label_check("x label", gp_widget_graph_x_label_get(graph), t->x_label))
		ret = TST_FAILED;

	if (label_check("y label", gp_widget_graph_y_label_get(graph), t->y_label))
		ret = TST_FAILED;

exit:
	gp_widget_free(graph);
	return ret;
}

static struct json_tcase json_bar = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"graph\", \"style\": \"bar\"}}",
	.style = GP_WIDGET_GRAPH_BAR,
	.flags = 0,
};

static struct json_tcase json_bar_error_bars = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"graph\", \"style\": \"bar\", \"error_bars\": true}}",
	.style = GP_WIDGET_GRAPH_BAR,
	.flags = GP_WIDGET_GRAPH_ERROR_BARS,
};

static struct json_tcase json_bar_error_bars_false = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"graph\", \"style\": \"bar\", \"error_bars\": false}}",
	.style = GP_WIDGET_GRAPH_BAR,
	.flags = 0,
};

static struct json_tcase json_stem = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"graph\", \"style\": \"stem\", \"error_bars\": true}}",
	.style = GP_WIDGET_GRAPH_STEM,
	.flags = GP_WIDGET_GRAPH_ERROR_BARS,
};

static struct json_tcase json_step = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"graph\", \"style\": \"step\"}}",
	.style = GP_WIDGET_GRAPH_STEP,
	.flags = 0,
};

static struct json_tcase json_labels = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"graph\", \"x_label\": \"Days\", \"y_label\": \"Passes\"}}",
	.x_label = "Days",
	.y_label = "Passes",
};

static struct json_tcase json_labels_empty = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"type\": \"graph\", \"x_label\": \"\", \"y_label\": \"\"}}",
};

const struct tst_suite tst_suite = {
	.suite_name = "graph testsuite",
	.tests = {
		{.name = "graph new free",
		 .tst_fn = graph_new_free,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph points set fewer",
		 .tst_fn = graph_points_set_fewer,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph points set more",
		 .tst_fn = graph_points_set_more,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph points set replace",
		 .tst_fn = graph_points_set_replace,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph points set zero",
		 .tst_fn = graph_points_set_zero,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph points copy fewer",
		 .tst_fn = graph_points_copy_fewer,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph points copy exact",
		 .tst_fn = graph_points_copy_exact,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph points copy grow",
		 .tst_fn = graph_points_copy_grow,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph points copy zero",
		 .tst_fn = graph_points_copy_zero,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph flags",
		 .tst_fn = graph_flags,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph style names",
		 .tst_fn = graph_style_names},

		{.name = "graph JSON bar",
		 .tst_fn = graph_json,
		 .data = &json_bar,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph JSON bar error_bars",
		 .tst_fn = graph_json,
		 .data = &json_bar_error_bars,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph JSON bar error_bars false",
		 .tst_fn = graph_json,
		 .data = &json_bar_error_bars_false,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph JSON stem",
		 .tst_fn = graph_json,
		 .data = &json_stem,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph JSON step",
		 .tst_fn = graph_json,
		 .data = &json_step,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph labels",
		 .tst_fn = graph_labels,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph JSON labels",
		 .tst_fn = graph_json,
		 .data = &json_labels,
		 .flags = TST_CHECK_MALLOC},

		{.name = "graph JSON labels empty",
		 .tst_fn = graph_json,
		 .data = &json_labels_empty,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
