@ include source.t
/*
 * Copyright (C) 2009 - 2012 Jiri Dluhos <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009 - 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

#include <core/gp_transform.h>
#include <core/gp_get_put_pixel.h>

#include <gfx/gp_line.h>
#include <gfx/gp_hline.h>
#include <gfx/gp_polygon.h>

/*
 * Line endpoints and slope dx/dy.
 */
struct gp_line {
	gp_coord y1, y2;
	gp_coord x1, x2;
	gp_coord dx;
	gp_coord dy;
};

static inline gp_coord get_x(const gp_coord *points, unsigned int i)
{
	return points[2*i];
}

static inline gp_coord get_y(const gp_coord *points, unsigned int i)
{
	return points[2*i+1];
}

/*
 * We shorten all lines by 1px at the end in order to get odd number of edges
 * on each scanline.
 */
static unsigned int init_lines(const gp_coord *points, unsigned int nvert,
                               struct gp_line *lines)
{
	gp_coord lx = get_x(points, nvert-1);
	gp_coord ly = get_y(points, nvert-1);
	unsigned int i, c = 0;

	for (i = 0; i < nvert; i++) {
		gp_coord cx = get_x(points, i);
		gp_coord cy = get_y(points, i);

		if (cy == ly)
			goto next;

		if (cy > ly) {
			lines[c].y1 = ly;
			lines[c].y2 = cy;

			lines[c].x1 = lx;
			lines[c].x2 = cx;

			lines[c].dx = cx - lx;
			lines[c].dy = cy - ly;
		} else {
			lines[c].y1 = cy;
			lines[c].y2 = ly;

			lines[c].x1 = cx;
			lines[c].x2 = lx;

			lines[c].dx = lx - cx;
			lines[c].dy = ly - cy;
		}

		lines[c].y2--;

		c++;
	next:
		lx = cx;
		ly = cy;
	}

	return c;
}

struct hline {
	gp_coord lx;
	gp_coord rx;
};

/*
 * Algorithm to find intersections for a given Y and a set of lines.
 *
 * For each line we find left and right intersection. These two points can be
 * quite far for lines that are nearly horizontal.
 *
 * This is easily computed as:
 *
 * - find Y relative to the line start
 * - for a given line we increase and decrease the given Y by a bit less than 0.5
 *   which yiels two points on Y that are then mapped to two points on a line on X
 *
 * There is also a special case, we have to make sure that start of each line
 * does not overshoot out of the polygon, which happens for lines that are
 * nearly horizontal. In this case we choose the exact middle of the pixel for
 * both coordinates, which means that we omit a few pixels which is fixed later
 * on.
 */
static unsigned int find_intersections(const struct gp_line *lines, unsigned int nvert,
                                       gp_coord y, struct hline *hlines)
{
	unsigned int i;
	unsigned int c = 0;

	for (i = 0; i < nvert; i++) {

		if (y < lines[i].y1 || y > lines[i].y2)
			continue;

		gp_coord ry = y - lines[i].y1;

		/*
		 * With this we effectively shorten the upper part of the edge
		 * so that it does not overshoot out of the polygon.
		 */
		if (y == lines[i].y1) {
			gp_coord mx = lines[i].x1 + 1.00 * lines[i].dx * ry / lines[i].dy + 0.5;
			hlines[c].lx = mx;
			hlines[c].rx = mx;
			c++;
			continue;
		}

		gp_coord lx = lines[i].x1 + 1.00 * lines[i].dx * (ry - 0.49999) / lines[i].dy + 0.5;
		gp_coord rx = lines[i].x1 + 1.00 * lines[i].dx * (ry + 0.49999) / lines[i].dy + 0.5;

		if (lx < rx) {
			hlines[c].lx = lx;
			hlines[c].rx = rx;
		} else {
			hlines[c].lx = rx;
			hlines[c].rx = lx;
		}

		c++;
	}

	return c;
}

struct scanline {
	gp_coord lx;
	gp_coord rx;
};

static unsigned int compute_scanlines(const struct gp_line *lines, unsigned int nvert,
                                      gp_coord y, struct scanline *scanlines)
{
	struct hline hlines[nvert];
	unsigned int i, j, cnt = find_intersections(lines, nvert, y, hlines);

	/* Bubble sort */
	for (i = 0; i < cnt; i++) {
		for (j = i+1; j < cnt; j++) {
			if (hlines[i].lx > hlines[j].lx ||
			    ((hlines[i].lx == hlines[j].lx) &&
			     (hlines[i].rx > hlines[j].rx))) {
				struct hline l = hlines[i];
				hlines[i] = hlines[j];
				hlines[j] = l;
			}
		}
	}

	unsigned int c = 0;

	for (i = 0; i < cnt-1; i+=2) {
		scanlines[c].lx = hlines[i].lx;
		scanlines[c].rx = hlines[i+1].rx;
		c++;
	}

	return c;
}

/*
 * This is more or less Bresenham that only draws line segment on a single
 * scanline y0.
 */
static gp_coord find_edge(gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1)
{
	gp_coord dx = GP_ABS(x1 - x0);
	gp_coord dy = GP_ABS(y1 - y0);
	gp_coord ex = x0;
	gp_coord x_dir = x1 < x0 ? -1 : 1;

	int err = 2*dy - dx - dy/2;

	while (err <= 0) {
		err += 2*dy;
		ex += x_dir;
	}

	return ex;
}

/*
 * Draw horizontal lines and edges around corner points separatelly.
 *
 * - Horizontal lines are removed from the list of edges
 * - We shorten the edges on each side of the Y axis
 */
static void draw_edges_hlines(gp_pixmap *pixmap, const gp_coord *points,
                              unsigned int nvert, gp_pixel pixel)
{
	gp_coord lx = get_x(points, nvert-1);
	gp_coord ly = get_y(points, nvert-1);
	unsigned int i;

	for (i = 0; i < nvert; i++) {
		gp_coord cx = get_x(points, i);
		gp_coord cy = get_y(points, i);

		if (cy == ly) {
			gp_hline_raw(pixmap, cx, lx, cy, pixel);
		} else {
			gp_coord ex;

			ex = find_edge(cx, cy, lx, ly);
			gp_hline(pixmap, cx, ex, cy, pixel);

			ex = find_edge(lx, ly, cx, cy);
			gp_hline(pixmap, lx, ex, ly, pixel);
		}

		lx = cx;
		ly = cy;
	}
}

@ for ps in pixelsizes:
static void fill_inner_polygon_{{ ps.suffix }}(gp_pixmap *pixmap, unsigned int nvert,
                         const gp_coord *xy, gp_pixel pixel)
{
	unsigned int i;
	struct gp_line lines[nvert];
	gp_coord y;

	gp_coord ymin = INT_MAX, ymax = -INT_MAX;
	for (i = 0; i < nvert; i++) {
		ymax = GP_MAX(ymax, get_y(xy, i));
		ymin = GP_MIN(ymin, get_y(xy, i));
	}

	unsigned int nlines = init_lines(xy, nvert, lines);

	for (y = ymin+1; y < ymax; y++) {
		struct scanline scanlines[nlines];
		unsigned int cnt;

		cnt = compute_scanlines(lines, nlines, y, scanlines);

		for (i = 0; i < cnt; i++)
			gp_hline_raw_{{ ps.suffix }}(pixmap, scanlines[i].lx, scanlines[i].rx, y, pixel);
	}
}

@ end
@
void gp_fill_polygon_raw(gp_pixmap *pixmap, unsigned int nvert,
                         const gp_coord *xy, gp_pixel pixel)
{

	switch (nvert) {
	case 1:
		gp_putpixel(pixmap, xy[0], xy[1], pixel);
		return;
	case 2:
		gp_line(pixmap, xy[0], xy[1], xy[2], xy[3], pixel);
		return;
	default:
	break;
	}

	GP_FN_PER_BPP_PIXMAP(fill_inner_polygon, pixmap, pixmap,
                             nvert, xy, pixel);

	draw_edges_hlines(pixmap, xy, nvert, pixel);
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
