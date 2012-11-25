#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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

void GP_FillPolygon_Raw(GP_Context *context, int vertex_count,
	const GP_Coord *xy, GP_Pixel pixel)
{
	float ymin = HUGE_VALF, ymax = -HUGE_VALF;
	struct GP_PolygonEdge *edge;
	struct GP_PolygonEdge edges[vertex_count];

	int i;
	for (i = 0; i < vertex_count - 1; i++) {
		edge = edges + i;
		GP_InitEdge(edge, xy[2*i], xy[2*i + 1],
			xy[2*i + 2], xy[2*i + 3]);
		ymin = fminf(ymin, edge->ymin);
		ymax = fmaxf(ymax, edge->ymax);
	}

	/* the last edge (from the last point to the first one) */
	edge = edges + vertex_count - 1;
	GP_InitEdge(edge, xy[2*i], xy[2*i + 1], xy[0], xy[1]);

	float intersections[vertex_count];
	int y;
	for (y = (int) ymin; y < (int) ymax; y++) {
		int inter_count = GP_ComputeScanline(intersections, edges, vertex_count, y + 0.5f);
		for (i = 0; i < inter_count; i+=2) {
			GP_HLine_Raw(context, intersections[i], intersections[i + 1], y, pixel);
		}
	}
}