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
#include <limits.h>

#include "core/GP_Transform.h"
#include "core/GP_GetPutPixel.h"

#include "GP_Line.h"
#include "GP_HLine.h"
#include "GP_Polygon.h"

/* "almost equality" for float coordinates */
#define GP_COORDS_ALMOST_EQUAL(a,b)	(fabsf(a-b) < 0.00001f)

/*
 * Edge state. Every edge proceeds from READY to ACTIVE and then FINISHED.
 * Numeric values reflect sorting priority (ACTIVE < READY < FINISHED).
 */
#define EDGE_FINISHED	2
#define EDGE_READY	1
#define EDGE_ACTIVE	0

/* Working record about an edge. */
struct GP_Edge {
	int state;	/* edge state */
	float x;		/* X coordinate of the working point */
	int y;		/* Y coordinate of the working point */
	int dy;		/* vertical size */
	float dxy;	/* dx/dy */
};

/* Initializes the edge structure. */
static void GP_InitEdge(struct GP_Edge *e, GP_Coord x1, GP_Coord y1,
	GP_Coord x2, GP_Coord y2)
{
	GP_ASSERT(y1 != y2, "horizontal edges not allowed here");

	/* initialize the working point to the top point of the edge */
	if (y1 < y2) {
		e->x = x1;
		e->y = y1;
	} else {
		e->x = x2;
		e->y = y2;
	}

	e->dy = GP_ABS(y2 - y1);
	e->dxy = (float)(x2 - x1)/(y2 - y1);
	e->state = EDGE_READY;
}

/* Type of a callback function to be passed to qsort(). */
typedef int (*GP_SortCallback)(const void *, const void *);

/*
 * Compares two edges. Used for initial sorting of the edges.
 * Edges are sorted by Y first, then by X, then by DXY.
 * Returns -1 if e1<e2, +1 if e1>e2, 0 if e1==e2.
 */
static int GP_CompareEdgesInitial(struct GP_Edge *e1, struct GP_Edge *e2)
{
	if (e1->y < e2->y) return -1;
	if (e1->y > e2->y) return 1;

	if (e1->x < e2->x) return -1;
	if (e1->x > e2->x) return 1;

	if (e1->dxy < e2->dxy) return -1;
	if (e1->dxy > e2->dxy) return 1;

	return 0;
}

/*
 * Compares two edges. Used for in-run sorting.
 * Edges are sorted by state (ACTIVE < READY < FINISHED), then by X,
 * then by DXY.
 * Returns -1 if e1<e2, +1 if e1>e2, 0 if e1==e2.
 */
static int GP_CompareEdgesRuntime(struct GP_Edge *e1, struct GP_Edge *e2)
{
	if (e1->state < e2->state) return -1;
	if (e1->state > e2->state) return 1;

	if (e1->x < e2->x) return -1;
	if (e1->x > e2->x) return 1;

	if (e1->dxy < e2->dxy) return -1;
	if (e1->dxy > e2->dxy) return 1;

	return 0;
}

typedef struct {
	GP_Coord x, y;
} GP_Point;

void GP_FillPolygon_Raw(GP_Context *context, unsigned int nvert,
                        const GP_Coord *xy, GP_Pixel pixel)
{
	unsigned int i;
	struct GP_Edge *e;

	if (nvert < 3)
		return;		/* not enough vertices */
	
	GP_Point const *vert = (GP_Point const *) xy;

	/* find first and last scanline */
	GP_Coord ymin = INT_MAX, ymax = -INT_MAX;
	for (i = 0; i < nvert; i++) {
		ymax = GP_MAX(ymax, vert[i].y);
		ymin = GP_MIN(ymin, vert[i].y);
	}
	
	/* build a list of edges */
	struct GP_Edge edges[nvert];
	unsigned int nedges = 0;		/* number of edges in list */
	for (i = 0; i < nvert; i++) {

		/*
		 * next vertex index (wraps to 0 at end to connect
		 * the last vertex with the first one)
		 */
		unsigned int nexti = (i+1) % nvert;

		/* skip horizontal edges */
		if (vert[i].y == vert[nexti].y) {
			continue;
		}

		GP_InitEdge(edges + nedges,
			vert[i].x, vert[i].y,
			vert[nexti].x, vert[nexti].y);
		e = edges + nedges;
		
		nedges++;
	}

	if (nedges < 2)
		return;		/* not really a polygon */
	
	for (i = 1; i < nedges; i++) {
		e = edges + i;
		struct GP_Edge *prev_e = edges + i - 1;
		if (GP_COORDS_ALMOST_EQUAL(prev_e->x + prev_e->dy*prev_e->dxy, e->x)) {
			prev_e->dy--;
		}
		else if (GP_COORDS_ALMOST_EQUAL(e->x + e->dy*e->dxy, prev_e->x)) {
			e->dy--;
		}
	}

	/* initially sort edges by Y, then X */
	qsort(edges, nedges, sizeof(struct GP_Edge),
	      (GP_SortCallback) GP_CompareEdgesInitial);

	for (i = 0; i < nedges; i++) {
		e = edges + i;
	}
	
	/*
	 * for each scanline, compute intersections with all edges
	 * and draw a horizontal line segment between the intersections.
	 */
	float inter[nedges];
	unsigned int ninter;
	int y = ymin;
	for (y = ymin; y <= ymax; y++) {

		/* mark edges we have just reached as active */
		for (i = 0; i < nedges; i++) {
			e = edges + i;
			if (e->state == EDGE_READY && (y == e->y)) {
				e->state = EDGE_ACTIVE;
			}
		}
		qsort(edges, nedges, sizeof(struct GP_Edge),
		      (GP_SortCallback) GP_CompareEdgesRuntime);
		
		/* record intersections with active edges */
		ninter = 0;
		for (i = 0; i < nedges; i++) {
			e = edges + i;
			if (e->state == EDGE_ACTIVE) {
				inter[ninter++] = e->x;
			}
		}
		
		/* draw each even range between intersections */
		for (i = 0; i < ninter; i += 2) {
			float start = inter[i];

			/* odd number of intersections - skip last */
			if (i+1 == ninter)
				break;

			float end = inter[i+1];
			GP_HLine_Raw(context, start, end, y, pixel);
		}

		/* check and mark edges we are done with */
		for (i = 0; i < nedges; i++) {
			e = edges + i;
			if (e->state == EDGE_ACTIVE && e->dy == 0) {
				e->state = EDGE_FINISHED;
			}
		}
		qsort(edges, nedges, sizeof(struct GP_Edge),
		      (GP_SortCallback) GP_CompareEdgesRuntime);
		
		/* update active edges for next step */
		for (i = 0; i < nedges; i++) {
			e = edges + i;
			if (e->state == EDGE_ACTIVE) {
				e->x += e->dxy;
				e->dy--;
			}
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
