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

#include "GP_Gfx.h"

#include <limits.h>
#include <stdlib.h>

struct GP_PolygonEdge {
	GP_Coord startx, starty;
	GP_Coord endx, endy;
	GP_Coord dx, dy;
};

struct GP_Polygon {
	struct GP_PolygonEdge *edges;
	GP_Coord edge_count;
	GP_Coord ymin, ymax;
};

#define GP_POLYGON_INITIALIZER { \
	.edges = NULL, \
	.edge_count = 0, \
	.ymin = INT_MAX, \
	.ymax = 0 \
}

static void GP_InitEdge(struct GP_PolygonEdge *edge,
		GP_Coord x1, GP_Coord y1, GP_Coord x2, GP_Coord y2)
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

	edge->dx = edge->endx - edge->startx;
	edge->dy = edge->endy - edge->starty;
}

static void GP_AddEdge(struct GP_Polygon *poly, GP_Coord x1, GP_Coord y1,
                       GP_Coord x2, GP_Coord y2)
{
	struct GP_PolygonEdge *edge = poly->edges + poly->edge_count;
	
	poly->edge_count++;

	GP_InitEdge(edge, x1, y1, x2, y2);

	poly->ymin = GP_MIN(poly->ymin, edge->starty);
	poly->ymax = GP_MAX(poly->ymax, edge->endy);
}

static GP_Coord GP_CompareEdges(const void *edge1, const void *edge2)
{
	struct GP_PolygonEdge *e1 = (struct GP_PolygonEdge *) edge1;
	struct GP_PolygonEdge *e2 = (struct GP_PolygonEdge *) edge2;

	if (e1->starty > e2->starty)
		return 1;
	if (e1->starty > e2->starty)
		return -1;

	if (e1->startx > e2->startx)
		return 1;
	if (e1->startx < e2->startx)
		return -1;

	return 0;
}

static void GP_LoadPolygon(struct GP_Polygon *poly, GP_Coord vertex_count,
		const GP_Coord *xy)
{
	poly->edge_count = 0;
	poly->edges = calloc(sizeof(struct GP_PolygonEdge),
			vertex_count);

	GP_Coord i;
	GP_Coord coord_count = 2*vertex_count - 2;
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

	/* sort edges */
	qsort(poly->edges, poly->edge_count, sizeof(struct GP_PolygonEdge),
		GP_CompareEdges);
}

static void GP_ResetPolygon(struct GP_Polygon *poly)
{
	free(poly->edges);
	poly->edges = NULL;
	poly->edge_count = 0;
	poly->ymin = INT_MAX;
	poly->ymax = 0;
}

/*
 * Finds an X coordinate of an GP_Coordersection of the edge
 * with the given Y line.
 */
static inline GP_Coord GP_FindIntersection(GP_Coord y, const struct GP_PolygonEdge *edge)
{
	GP_Coord edge_y = y - edge->starty;		/* Y relative to the edge */
	GP_Coord x = edge->startx;

	if (edge->dx > 0) {
		while (edge->startx*edge->dy + edge_y*edge->dx > x*edge->dy)
			x++;
	} else {
		while (edge->startx*edge->dy + edge_y*edge->dx < x*edge->dy)
			x--;
	}

	return x;
}

void GP_FillPolygon(GP_Context *context, GP_Coord vertex_count, const GP_Coord *xy,
		GP_Pixel pixel)
{
	struct GP_Polygon poly = GP_POLYGON_INITIALIZER;

	GP_LoadPolygon(&poly, vertex_count, xy);

	GP_Coord y, startx, endx;
	GP_Coord startx_prev = -INT_MAX;
	GP_Coord endx_prev = INT_MAX;

	for (y = poly.ymin; y <= poly.ymax; y++) {
		startx = INT_MAX;
		endx = 0;

		GP_Coord i;
		for (i = 0; i < poly.edge_count; i++) {
			struct GP_PolygonEdge *edge = poly.edges + i;

			if (y < edge->starty || y > edge->endy)
				continue;

			GP_Coord GP_Coorder = GP_FindIntersection(y, edge);

			startx = GP_MIN(startx, GP_Coorder);
			endx = GP_MAX(endx, GP_Coorder);

			if (y != edge->endy) {
				GP_Coorder = GP_FindIntersection(y + 1, edge);
				startx = GP_MIN(startx, GP_Coorder);
				endx = GP_MAX(endx, GP_Coorder);
			}
		}

		GP_HLine(context, startx, endx, y, pixel);

		startx_prev = startx;
		endx_prev = endx;
	}

	GP_ResetPolygon(&poly);
}
