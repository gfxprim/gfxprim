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

struct GP_PolygonEdge {
	int startx, starty;
	int endx, endy;
	int dx, dy, sx;
	struct GP_PolygonEdge *next;
};

struct GP_Polygon {
	struct GP_PolygonEdge *edges;
	int ymin, ymax;
};

static struct GP_PolygonEdge *GP_NewEdge(int x1, int y1, int x2, int y2)
{
	struct GP_PolygonEdge *edge = malloc(sizeof(*edge));
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

	return edge;
}

static void GP_InsertEdge(struct GP_Polygon *poly, struct GP_PolygonEdge *new_edge)
{
	struct GP_PolygonEdge *cur_edge = poly->edges;

	poly->ymin = GP_MIN(poly->ymin, new_edge->starty);
	poly->ymax = GP_MAX(poly->ymax, new_edge->endy);

	if (cur_edge == NULL || cur_edge->starty > new_edge->starty) {
		new_edge->next = cur_edge;
		poly->edges = new_edge;
		return;
	}

	for (;;) {
		if (cur_edge->next == NULL || cur_edge->next->starty > new_edge->starty) {
			new_edge->next = cur_edge->next;
			cur_edge->next = new_edge;
			return;
		}
		cur_edge = cur_edge->next;
	}
}

static struct GP_Polygon *GP_NewPolygon(void)
{
	struct GP_Polygon *poly = malloc(sizeof(struct GP_Polygon));
	poly->edges = NULL;
	poly->ymin = INT_MAX;
	poly->ymax = 0;
	return poly;
}

static void GP_LoadPolygon(struct GP_Polygon *poly, int vertex_count, int *xy)
{
	int i;
	int coord_count = 2*vertex_count - 2;
	for (i = 0; i < coord_count; i+=2) {


		if (xy[i+1] == xy[i+3]) {
			continue;	/* skip horizontal edges */
		}

		struct GP_PolygonEdge *edge = GP_NewEdge(xy[i],
				xy[i+1], xy[i+2], xy[i+3]);
		
		GP_InsertEdge(poly, edge);
	}

	/* add the closing edge */
	if (xy[1] != xy[i+1]) {
		struct GP_PolygonEdge *edge = GP_NewEdge(xy[i], xy[i+1],
			xy[0], xy[1]);
		GP_InsertEdge(poly, edge);
	}
}

static void GP_FreePolygon(struct GP_Polygon *poly)
{
	struct GP_PolygonEdge *edge = poly->edges;
	for (;;) {
		struct GP_PolygonEdge *next_edge = edge->next;
		free(edge);
		if (next_edge == NULL)
			break;
		edge = next_edge;
	}
	free(poly);
}

void GP_FillPolygon(GP_Context *context, int vertex_count, int *xy, GP_Pixel pixel)
{
	struct GP_Polygon *poly = GP_NewPolygon();
	GP_LoadPolygon(poly, vertex_count, xy);

	int y;
	for (y = poly->ymin; y <= poly->ymax; y++) {
		int startx = INT_MAX;
		int endx = 0;

		struct GP_PolygonEdge *edge;
		for (edge = poly->edges; edge; edge = edge->next) {
			if (y >= edge->starty && y <= edge->endy) {
				int edge_y = y - edge->starty;
				int edge_x;
				for (edge_x = edge->sx > 0 ? edge->startx: edge->endx;
					edge->startx*edge->dy + edge->sx*edge_y*edge->dx - edge_x*edge->dy > 0;
					edge_x++);
				startx = GP_MIN(startx, edge_x);
				endx = GP_MAX(endx, edge_x);
			}
		}

		GP_HLine(context, startx, endx, y, pixel);
	}

	GP_FreePolygon(poly);
}
