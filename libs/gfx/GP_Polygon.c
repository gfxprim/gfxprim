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

/* A 2D point specified by gp_coord coordinates. */
typedef struct {
	gp_coord x, y;
} gp_point;

/* "almost equality" for float coordinates */
#define GP_COORDS_ALMOST_EQUAL(a,b)	(fabsf((a)-(b)) < 0.0001f)

/*
 * Edge state. Every edge proceeds from READY to ACTIVE and then FINISHED.
 * HORIZONTAL is special (horizontal edges are handled separately).
 * Numeric values reflect sorting priority (ACTIVE is foremost).
 */
#define EDGE_HORIZONTAL	3
#define EDGE_FINISHED	2
#define EDGE_READY	1
#define EDGE_ACTIVE	0

/* Working record about an edge. */
struct edge {
	int state;	/* edge state */
	float x;		/* X coordinate of the working point */
	int y;		/* Y coordinate of the working point */
	int dy;		/* vertical size */
	float dxy;	/* dx/dy */
};

/* Initializes the edge structure. */
static void init_edge(struct edge *e, gp_point start, gp_point end)
{
	/* horizontal edges are a special case */
	if (start.y == end.y) {
		e->dy = 0;
		e->x = start.x;
		e->y = start.y;
		e->dxy = end.x - start.x;
		e->state = EDGE_HORIZONTAL;
		return;
	}

	/* initialize the working point to the top point of the edge */
	if (start.y < end.y) {
		e->x = (float) start.x;
		e->y = start.y;
	} else {
		e->x = (float) end.x;
		e->y = end.y;
	}

	e->dy = GP_ABS(end.y - start.y);

	e->dxy = (float)(end.x - start.x)/(end.y - start.y);
	e->state = EDGE_READY;

	/* Shorten each edge by one pixel at the bottom. This prevents
	 * every vertex point to be reported as two intersections.
	 * This also means causes all horizontal edges cut by one pixel,
	 * but we will fix this at the end by drawing them separately.
	 */
	e->dy--;
}

/* Type of a callback function to be passed to qsort(). */
typedef int (*gp_sort_callback)(const void *, const void *);

/*
 * Compares two edges. Used for initial sorting of the edges.
 * Edges are sorted by Y first, then by X, then by DXY.
 * Returns -1 if e1<e2, +1 if e1>e2, 0 if e1==e2.
 */
static int edges_compare_initial(struct edge *e1, struct edge *e2)
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
static int edges_compare_runtime(struct edge *e1, struct edge *e2)
{
	if (e1->state < e2->state) return -1;
	if (e1->state > e2->state) return 1;

	if (e1->x < e2->x) return -1;
	if (e1->x > e2->x) return 1;

	if (e1->dxy < e2->dxy) return -1;
	if (e1->dxy > e2->dxy) return 1;

	return 0;
}

void gp_fill_polygon_raw(gp_pixmap *pixmap, unsigned int nvert,
                         const gp_coord *xy, gp_pixel pixel)
{
	unsigned int i;
	struct edge *e;

	if (nvert < 3)
		return;		/* not enough vertices */

	gp_point const *vert = (gp_point const *) xy;

	/* find first and last scanline */
	gp_coord ymin = INT_MAX, ymax = -INT_MAX;
	for (i = 0; i < nvert; i++) {
		ymax = GP_MAX(ymax, vert[i].y);
		ymin = GP_MIN(ymin, vert[i].y);
	}

	/* build a list of edges */
	struct edge edges[nvert];
	unsigned int nedges = 0;		/* number of edges in list */
	for (i = 0; i < nvert; i++) {

		/*
		 * next vertex index (wraps to 0 at end to connect
		 * the last vertex with the first one)
		 */
		unsigned int nexti = (i+1) % nvert;

		/* add new edge record */
		e = edges + nedges++;
		init_edge(e, vert[i], vert[nexti]);
	}

	if (nedges < 2)
		return;		/* not really a polygon */

	/* initially sort edges by Y, then X */
	qsort(edges, nedges, sizeof(struct edge),
	      (gp_sort_callback) edges_compare_initial);

	/*
	 * for each scanline, compute intersections with all edges
	 * and draw a horizontal line segment between the intersections.
	 */
	float inter[nedges];
	unsigned int ninter;
	int y;
	for (y = ymin; y <= ymax; y++) {

		/* mark edges we have just reached as active */
		for (i = 0; i < nedges; i++) {
			e = edges + i;
			if (e->state == EDGE_READY && (y == e->y)) {
				e->state = EDGE_ACTIVE;
			}
		}
		qsort(edges, nedges, sizeof(struct edge),
		      (gp_sort_callback) edges_compare_runtime);

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
			gp_hline_raw(pixmap, start, end, y, pixel);
		}

		/* update active edges for next step */
		for (i = 0; i < nedges; i++) {
			e = edges + i;
			if (e->state == EDGE_ACTIVE) {
				if (e->dy == 0) {
					e->state = EDGE_FINISHED;
				} else {
					e->x += e->dxy;
					e->dy--;
				}
			}
		}
	}

	/* finishing touch: draw all horizontal edges that were skipped
	 * in the main loop
	 */
	for (i = 0; i < nedges; i++) {
		e = edges + i;
		if (e->state == EDGE_HORIZONTAL) {
			gp_hline_raw(pixmap, e->x, e->x + e->dxy, e->y,
				     pixel);
		}
	}
}

void gp_fill_polygon(gp_pixmap *pixmap, unsigned int vertex_count,
                     const gp_coord *xy, gp_pixel pixel)
{
	unsigned int i;
	gp_coord xy_copy[2 * vertex_count];

	for (i = 0; i < vertex_count; i++) {
		unsigned int x = 2 * i;
		unsigned int y = 2 * i + 1;

		xy_copy[x] = xy[x];
		xy_copy[y] = xy[y];
		GP_TRANSFORM_POINT(pixmap, xy_copy[x], xy_copy[y]);
	}

	gp_fill_polygon_raw(pixmap, vertex_count, xy_copy, pixel);
}

void gp_polygon_raw(gp_pixmap *pixmap, unsigned int vertex_count,
                    const gp_coord *xy, gp_pixel pixel)
{
	unsigned int i;

	gp_coord prev_x = xy[2 * vertex_count - 2];
	gp_coord prev_y = xy[2 * vertex_count - 1];

	for (i = 0; i < vertex_count; i++) {
		gp_coord x = xy[2 * i];
		gp_coord y = xy[2 * i + 1];

		gp_line_raw(pixmap, prev_x, prev_y, x, y, pixel);

		prev_x = x;
		prev_y = y;
	}
}


void gp_polygon(gp_pixmap *pixmap, unsigned int vertex_count,
                const gp_coord *xy, gp_pixel pixel)
{
	unsigned int i;

	gp_coord prev_x = xy[2 * vertex_count - 2];
	gp_coord prev_y = xy[2 * vertex_count - 1];

	GP_TRANSFORM_POINT(pixmap, prev_x, prev_y);

	for (i = 0; i < vertex_count; i++) {
		gp_coord x = xy[2 * i];
		gp_coord y = xy[2 * i + 1];

		GP_TRANSFORM_POINT(pixmap, x, y);

		gp_line_raw(pixmap, prev_x, prev_y, x, y, pixel);

		prev_x = x;
		prev_y = y;
	}
}
