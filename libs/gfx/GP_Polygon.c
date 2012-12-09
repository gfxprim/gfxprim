/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009 - 2012 Jiri Dluhos <jiri.bluebear.dluhos@gmail.com>    *
 * Copyright (C) 2009 - 2012 Cyril Hrubis <metan@ucw.cz>                     *
 *                                                                           *
 *****************************************************************************/

#define _ISOC99_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "core/GP_Transform.h"
#include "core/GP_GetPutPixel.h"

#include "GP_Line.h"
#include "GP_HLine.h"
#include "GP_Polygon.h"

/* A single edge of the polygon. */
struct GP_PolygonEdge {
	float x1, y1, x2, y2;
	float ymin, ymax;
	float dx_by_dy;		/* dx/dy (0 for horizontal edges) */
};

/* Threshod value for delta y; edges with dy smaller than this are considered horizontal. */
const float GP_HORIZ_DY_THRESHOLD = 0.00001f;

/* Initializes the polygon_edge structure. */
static void GP_InitEdge(struct GP_PolygonEdge *edge, float x1, float y1,
	float x2, float y2)
{
	edge->x1 = x1;
	edge->y1 = y1;
	edge->x2 = x2;
	edge->y2 = y2;
	edge->ymin = fminf(edge->y1, edge->y2);
	edge->ymax = fmaxf(edge->y1, edge->y2);

	/* horizontal (or almost horizontal) edges are a special case */
	if ((edge->ymax - edge->ymin) < GP_HORIZ_DY_THRESHOLD) {
		edge->dx_by_dy = 0.0;	/* not meaningful */
		return;
	}

	float dx = (edge->x2 - edge->x1);
	float dy = (edge->y2 - edge->y1);
	edge->dx_by_dy = dx / dy;
}

/* Computes an intersection of the specified scanline with the given edge.
 * If successful, returns 1 and stores the resulting X coordinate into result_x.
 * If failed (the edge does not intersect), 0 is returned.
 * Horizontal edges are considered to never intersect.
 */
static int GP_ComputeIntersection(float *result_x, struct GP_PolygonEdge *edge, float y)
{
	if (y<edge->ymin || y>edge->ymax)
		return 0;		/* outside the edge Y range */

	if (edge->ymax - edge->ymin < GP_HORIZ_DY_THRESHOLD)
		return 0;		/* ignore horizontal edges */

	*result_x = edge->x1 + (y-edge->y1)*edge->dx_by_dy;

	return 1;
}

/* Sorting callback. Compares two floats and returns -1 if A<B,
 * +1 if A>B, 0 if they are equal.
 */
static int GP_CompareFloats(const void *ptr_a, const void *ptr_b)
{
	float a = ((float *) ptr_a)[0];
	float b = ((float *) ptr_b)[0];

	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

/* Computes intersections of the y coordinate with all edges,
 * writing the X coordinates of the intersections, sorted by X coordinate,
 * into 'results'.
 */
static int GP_ComputeScanline(float *results, struct GP_PolygonEdge *edges,
	size_t count, float y)
{
	unsigned int edge_index = 0;
	int result_index = 0;

	for (; edge_index<count; edge_index++) {

		struct GP_PolygonEdge *edge = edges + edge_index;
		float x;

		if (GP_ComputeIntersection(&x, edge, y)) {
			results[result_index++] = x;
		}
	}

	qsort(results, result_index, sizeof(float), GP_CompareFloats);

	return result_index;
}

void GP_FillPolygon_Raw(GP_Context *context, unsigned int vertex_count,
                        const GP_Coord *xy, GP_Pixel pixel)
{
	float ymin = HUGE_VALF, ymax = -HUGE_VALF;
	struct GP_PolygonEdge *edge;
	struct GP_PolygonEdge edges[vertex_count];


	/* Build edge structures for each vertex-vertex connection.
	 * NOTE: Each vertex is in fact in the middle of the pixel, so
	 * add 0.5 to both coordinates.
	 */
	int i;
	for (i = 0; i < (int)vertex_count - 1; i++) {
		edge = edges + i;
		GP_InitEdge(edge,
			0.5f + xy[2*i], 0.5f + xy[2*i + 1],
			0.5f + xy[2*i + 2], 0.5f + xy[2*i + 3]);
		ymin = fminf(ymin, edge->ymin);
		ymax = fmaxf(ymax, edge->ymax);
	}

	/* the last edge (from the last point to the first one) */
	edge = edges + vertex_count - 1;
	GP_InitEdge(edge,
		0.5f + xy[2*i], 0.5f + xy[2*i + 1],
		0.5f + xy[0], 0.5f + xy[1]);

	/* For each scanline, compute intersections with all edges
	 * and draw a horizontal line segment between the intersections.
	 */
	float intersections[vertex_count];
	int y;
	for (y = (int) ymin; y <= (int) ymax; y++) {
		int inter_count = GP_ComputeScanline(intersections, edges, vertex_count, y + 0.5f);

		i = 0;
		for (;;) {
			if (i >= inter_count) break;
			float start = intersections[i++];
			if (i >= inter_count) {

				/* a solo vertex or a single-point intersection */
				GP_PutPixel_Raw(context, start, y, pixel);
				break;
			}
			float end = intersections[i++];
			if (start == end) {

				/* two intersections - edge joint */
				if (i >= inter_count) break;
				end = intersections[i++];
			}
			GP_HLine_Raw(context, start, end, y, pixel);
		}
	}
}

void GP_FillPolygon(GP_Context *context, unsigned int vertex_count,
                    const GP_Coord *xy, GP_Pixel pixel)
{
	unsigned int i;
	GP_Coord xy_copy[2 * vertex_count];

	for (i = 0; i < vertex_count; i++) {
		unsigned int x = 2 * i;
		unsigned int y = 2 * i + 1;

		xy_copy[x] = xy[x];
		xy_copy[y] = xy[y];
		GP_TRANSFORM_POINT(context, xy_copy[x], xy_copy[y]);
	}

	GP_FillPolygon_Raw(context, vertex_count, xy_copy, pixel);
}

void GP_Polygon_Raw(GP_Context *context, unsigned int vertex_count,
                    const GP_Coord *xy, GP_Pixel pixel)
{
	unsigned int i;

	GP_Coord prev_x = xy[2 * vertex_count - 2];
	GP_Coord prev_y = xy[2 * vertex_count - 1];

	for (i = 0; i < vertex_count; i++) {
		GP_Coord x = xy[2 * i];
		GP_Coord y = xy[2 * i + 1];

		GP_Line_Raw(context, prev_x, prev_y, x, y, pixel);

		prev_x = x;
		prev_y = y;
	}
}


void GP_Polygon(GP_Context *context, unsigned int vertex_count,
                const GP_Coord *xy, GP_Pixel pixel)
{
	unsigned int i;

	GP_Coord prev_x = xy[2 * vertex_count - 2];
	GP_Coord prev_y = xy[2 * vertex_count - 1];

	GP_TRANSFORM_POINT(context, prev_x, prev_y);

	for (i = 0; i < vertex_count; i++) {
		GP_Coord x = xy[2 * i];
		GP_Coord y = xy[2 * i + 1];

		GP_TRANSFORM_POINT(context, x, y);

		GP_Line_Raw(context, prev_x, prev_y, x, y, pixel);

		prev_x = x;
		prev_y = y;
	}
}
