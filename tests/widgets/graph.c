// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
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

		{.name = NULL},
	}
};
