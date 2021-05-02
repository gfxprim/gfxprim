// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009 - 2012 Jiri Dluhos <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009 - 2012 Cyril Hrubis <metan@ucw.cz>
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
 * We shorten all lines but the ones that end on ymax by 1 on Y in order to
 * avoid odd number of intersections.
 */
static unsigned int init_lines(const gp_coord *points, unsigned int nvert,
                               struct gp_line *lines, gp_coord ymax)
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

		if (lines[c].y2 != ymax)
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
 * Algorithm to find intersections for a given set of lines.
 *
 * For each line we find left and right intersection on a given y.
 *
 * This is easily computed as:
 *
 * - find Y relative to the line start
 * - for a given line we increase and decrease the Y by a bit less than 0.5
 *   which yiels two points on Y that are then mapped to two points on a line on X
 */
static unsigned int find_intersections(const struct gp_line *lines, unsigned int nvert, gp_coord y, struct hline *hlines)
{
	unsigned int i;
	unsigned int c = 0;

	for (i = 0; i < nvert; i++) {

		if (y < lines[i].y1 || y > lines[i].y2)
			continue;

		gp_coord ry = y - lines[i].y1;
		gp_coord lx, rx;

		lx = lines[i].x1 + 1.00 * lines[i].dx * (ry - 0.49) / lines[i].dy + 0.5;
		rx = lines[i].x1 + 1.00 * lines[i].dx * (ry + 0.49) / lines[i].dy + 0.5;

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

static unsigned int compute_scanlines(const struct gp_line *lines, unsigned int nvert, gp_coord y, struct scanline *scanlines)
{
	struct hline hlines[nvert];
	unsigned int i, j, cnt = find_intersections(lines, nvert, y, hlines);
/*
	printf("HLINES ");
	for (i = 0; i < cnt; i++)
		printf("%i-%i ", hlines[i].lx, hlines[i].rx);
	printf("\n");
*/
	/* Bubble sort */
	for (i = 0; i < cnt; i++) {
		for (j = i+1; j < cnt; j++) {
			if (hlines[i].lx > hlines[j].lx) {
				struct hline l = hlines[i];
				hlines[i] = hlines[j];
				hlines[j] = l;
			}
		}
	}

	if (!cnt)
		return 0;

	unsigned int c = 0;

	for (i = 0; i < cnt-1; i+=2) {
		scanlines[c].lx = hlines[i].lx;
		scanlines[c].rx = hlines[i+1].rx;
		c++;
	}

	return c;
}

/*
 * We remove horizontal lines from the list, so have to draw them separately.
 */
static void draw_hlines(gp_pixmap *pixmap, const gp_coord *points,
                        unsigned int nvert, gp_pixel pixel)
{
	gp_coord lx = get_x(points, nvert-1);
	gp_coord ly = get_y(points, nvert-1);
	unsigned int i;

	for (i = 0; i < nvert; i++) {
		gp_coord cx = get_x(points, i);
		gp_coord cy = get_y(points, i);

		if (cy == ly)
			gp_hline_raw(pixmap, cx, lx, cy, pixel);

		lx = cx;
		ly = cy;
	}
}

void gp_fill_polygon_raw(gp_pixmap *pixmap, unsigned int nvert,
                         const gp_coord *xy, gp_pixel pixel)
{
	unsigned int i;
	struct gp_line lines[nvert];
	gp_coord y;

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

	draw_hlines(pixmap, xy, nvert, pixel);

	gp_coord ymin = INT_MAX, ymax = -INT_MAX;
	for (i = 0; i < nvert; i++) {
		ymax = GP_MAX(ymax, get_y(xy, i));
		ymin = GP_MIN(ymin, get_y(xy, i));
	}

	unsigned int nlines = init_lines(xy, nvert, lines, ymax);

	for (y = ymin; y <= ymax; y++) {
		struct scanline scanlines[nlines];
		unsigned int cnt;

		cnt = compute_scanlines(lines, nlines, y, scanlines);

		for (i = 0; i < cnt; i++) {
//			printf("%i-%i ", scanlines[i].lx, scanlines[i].rx);
			gp_hline_raw(pixmap, scanlines[i].lx, scanlines[i].rx, y, pixel);
		}
//		printf("\n");
	}

//	printf("\n\n\n");
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
