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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "GP.h"

#include <limits.h>
#include <stdlib.h>

struct GP_PolygonEdge {
	int startx, starty;
	int endx, endy;
	int dx, dy, sx;
};

struct GP_Polygon {
	struct GP_PolygonEdge *edges;
	int edge_count;
	int ymin, ymax;
};

#define GP_POLYGON_INITIALIZER { \
	.edges = NULL, \
	.edge_count = 0, \
	.ymin = INT_MAX, \
	.ymax = 0 \
}

static void GP_InitEdge(struct GP_PolygonEdge *edge,
		int x1, int y1, int x2, int y2)
{
	if (y1 < y2) {			/* coords are top-down, correct */
		edge->startx = x1;
		edge->starty = y1;
		edge->endx = x2;
		edge->endy = y2;
	} else {			/* coords are bottom-up, flip them */
		edge->startx = x2;
		edge->starty = y2;
		edge->endx = x1;
		edge->endy = y1;
	}

	edge->dx = abs(edge->endx - edge->startx);
	edge->dy = edge->endy - edge->starty;
	edge->sx = (edge->endx >= edge->startx) ? 1 : -1;
}

static void GP_AddEdge(struct GP_Polygon *poly, int x1, int y1, int x2, int y2)
{
	struct GP_PolygonEdge *edge = poly->edges + poly->edge_count;
	
	poly->edge_count++;

	GP_InitEdge(edge, x1, y1, x2, y2);

	poly->ymin = GP_MIN(poly->ymin, edge->starty);
	poly->ymax = GP_MAX(poly->ymax, edge->endy);
}

static int GP_CompareEdgeStartY(const void *edge1, const void *edge2)
{
	int y1 = ((struct GP_PolygonEdge *) edge1)->starty;
	int y2 = ((struct GP_PolygonEdge *) edge2)->starty;
	if (y1 > y2)
		return 1;
	else if (y1 == y2)
		return 0;
	else
		return -1;
}

static void GP_LoadPolygon(struct GP_Polygon *poly, int vertex_count,
		const int *xy)
{
	poly->edge_count = 0;
	poly->edges = calloc(sizeof(struct GP_PolygonEdge),
			vertex_count);

	int i;
	int coord_count = 2*vertex_count - 2;
	for (i = 0; i < coord_count; i+=2) { 

		/* add the edge, unless it is horizontal */
		if (xy[i+1] != xy[i+3]) {
			GP_AddEdge(poly, xy[i], xy[i+1], xy[i+2], xy[i+3]);
		}
	}

	/* add the closing edge, unless it is horizontal */
	if (xy[1] != xy[i+1]) {
		GP_AddEdge(poly, xy[i], xy[i+1], xy[0], xy[1]);
	}

	/* sort edges by their starting Y coordinate */
	qsort(poly->edges, poly->edge_count, sizeof(struct GP_PolygonEdge),
		GP_CompareEdgeStartY);
}

static void GP_ResetPolygon(struct GP_Polygon *poly)
{
	free(poly->edges);
	poly->edges = NULL;
	poly->edge_count = 0;
	poly->ymin = INT_MAX;
	poly->ymax = 0;
}

/* Finds an X coordinate of an intersection of the edge
 * with the given Y line.
 */
static inline int GP_FindIntersection(int y, const struct GP_PolygonEdge *edge)
{
	int edge_y = y - edge->starty;
	int edge_x;
	for (edge_x = edge->sx > 0 ? edge->startx : edge->endx;
		edge->startx*edge->dy + edge->sx*edge_y*edge->dx - edge_x*edge->dy > 0;
		edge_x++);
	return edge_x;
}

void GP_FillPolygon(GP_Context *context, int vertex_count, const int *xy,
		GP_Pixel pixel)
{
	struct GP_Polygon poly = GP_POLYGON_INITIALIZER;

	GP_LoadPolygon(&poly, vertex_count, xy);

	int y;
	for (y = poly.ymin; y <= poly.ymax; y++) {
		int startx = INT_MAX;
		int endx = 0;

		int i;
		for (i = 0; i < poly.edge_count; i++) {
			struct GP_PolygonEdge *edge = poly.edges + i;

			if (y < edge->starty || y > edge->endy)
				continue;

			int edge_x = GP_FindIntersection(y, edge);
			startx = GP_MIN(startx, edge_x);
			endx = GP_MAX(endx, edge_x);
		}

		GP_HLine(context, startx, endx, y, pixel);
	}

	GP_ResetPolygon(&poly);
}
