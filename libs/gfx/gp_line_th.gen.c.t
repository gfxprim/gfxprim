@ include source.t
/*
 * Thick line drawing algorithm.
 *
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <math.h>

#include <core/gp_common.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_fn_per_bpp.h>

#include <gfx/gp_hline.h>
#include <gfx/gp_line.h>
#include <gfx/gp_line_clip.h>

/*
 * Murphy thick line drawing algorithm.
 *
 * This is a modified Bresenham, we run two Bresenham line algorithms, outher
 * one that track the line and inner one that draws lines perpendicular to the
 * line we track. The outher Bresenham also tracks initial errors for the inner
 * one.
 *
 * We do a nice little trick on the top of the original algorithm, we only
 * track half of the line and paint it from both sides at the same time.
 *
 * See also http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 */

@ for ps in pixelsizes:
static void perp_dy_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int y0,
                  int dx, int dy, int errinit, int width, int winit,
		  int xstep, int ystep, gp_pixel pixval)
{
	int l, err, x, y;
	int threshold = dy - 2 * dx;

	x = x0;
	y = y0;
	l = dx + dy - winit;
	err = errinit;

	while (l <= width) {
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, pixval);
		if (err > threshold) {
			y-=ystep;
			err -= 2*dy;
			l += 2*dx;
		}
		err += 2*dx;
		x+=xstep;
		l += 2*dy;
	}

	x = x0;
	y = y0;
	err = -errinit;
	l = dx + dy + winit;

	while (l <= width) {
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, pixval);
		if (err > threshold) {
			y+=ystep;
			err -= 2*dy;
			l += 2*dx;
		}
		err += 2*dx;
		x-=xstep;
		l += 2*dy;
	}
}

static void line_dy_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int y0,
                                    int x1, int y1, gp_size r,
                                    gp_pixel pixval)
{
	if (y0 > y1) {
		GP_SWAP(y0, y1);
		GP_SWAP(x0, x1);
	}

	int dy = y1 - y0;
	int dx = GP_ABS(x1 - x0);

	int width = 4*r*sqrt(dx*dx + dy*dy);

	int err = 0;
	int perr = 0;
	int threshold = dy - 2*dx;

	int x = 0, y;
	int xstep = (x0 < x1) ? 1 : -1;

	for (y = 0; y <= (dy+1)/2; y++) {
		perp_dy_{{ ps.suffix }}(pixmap, x0+x, y0+y, dx, dy, perr, width, err, xstep, 1, pixval);
		perp_dy_{{ ps.suffix }}(pixmap, x1-x, y1-y, dx, dy, perr, width, err, -xstep, -1, pixval);

		if (err >= threshold) {
			x += xstep;
			err -= 2*dy;
			if (perr > threshold) {
				perp_dy_{{ ps.suffix }}(pixmap, x0+x, y0+y, dx, dy, perr - 2*dy + 2*dx, width, err, xstep, 1, pixval);
				perp_dy_{{ ps.suffix }}(pixmap, x1-x, y1-y, dx, dy, perr - 2*dy + 2*dx, width, err, -xstep, -1, pixval);
				perr -= 2*dy;
			}
			perr += 2*dx;
		}
		err += 2*dx;
	}

}

static void perp_dx_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int y0,
                  int dx, int dy, int errinit, int width, int winit,
		  int xstep, int ystep, gp_pixel pixval)
{
	int l, err, x, y;
	int threshold = dx - 2 * dy;

	x = x0;
	y = y0;
	l = dx + dy - winit;
	err = errinit;

	while (l <= width) {
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, pixval);
		if (err > threshold) {
			x-=xstep;
			err -= 2*dx;
			l += 2*dy;
		}
		err += 2*dy;
		y+=ystep;
		l += 2*dx;
	}

	x = x0;
	y = y0;
	err = -errinit;
	l = dx + dy + winit;

	while (l <= width) {
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x, y, pixval);
		if (err > threshold) {
			x+=xstep;
			err -= 2*dx;
			l += 2*dy;
		}
		err += 2*dy;
		y-=ystep;
		l += 2*dx;
	}
}

static void line_dx_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int y0,
                                    int x1, int y1, gp_size r,
                                    gp_pixel pixval)
{
	if (x0 > x1) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	int dx = x1 - x0;
	int dy = GP_ABS(y1 - y0);

	int width = 4*r*sqrt(dx*dx + dy*dy);

	int err = 0;
	int perr = 0;
	int threshold = dx - 2 * dy;

	int y = 0, x;
	int ystep = (y0 < y1) ? 1 : -1;

	for (x = 0; x <= (dx+1)/2; x++) {
		perp_dx_{{ ps.suffix }}(pixmap, x0+x, y0+y, dx, dy, perr, width, err, 1, ystep, pixval);
		perp_dx_{{ ps.suffix }}(pixmap, x1-x, y1-y, dx, dy, perr, width, err, -1, -ystep, pixval);

		if (err >= threshold) {
			y += ystep;
			err -= 2*dx;
			if (perr > threshold) {
				perp_dx_{{ ps.suffix }}(pixmap, x0+x, y0+y, dx, dy, perr - 2*dx + 2*dy, width, err, 1, ystep, pixval);
				perp_dx_{{ ps.suffix }}(pixmap, x1-x, y1-y, dx, dy, perr - 2*dx + 2*dy, width, err, -1, -ystep, pixval);
				perr -= 2*dx;
			}
			perr += 2*dy;
		}
		err += 2*dy;
	}
}

static void fill_rect_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int y0,
                                      int x1, int y1, gp_pixel pixval)
{
	gp_coord y;

	for (y = y0; y <= y1; y++)
		gp_hline_raw_{{ ps.suffix }}(pixmap, x0, x1, y, pixval);
}

void gp_line_th_raw_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int y0,
	int x1, int y1, gp_size r, gp_pixel pixval)
{
	if (!gp_line_clip(&x0, &y0, &x1, &y1, pixmap->w - 1, pixmap->h - 1))
		return;

	GP_ASSERT(x0 >= 0 && x0 <= (int) pixmap->w-1);
	GP_ASSERT(x1 >= 0 && x1 <= (int) pixmap->w-1);
	GP_ASSERT(y0 >= 0 && y0 <= (int) pixmap->h-1);
	GP_ASSERT(y1 >= 0 && y1 <= (int) pixmap->h-1);

	/* special cases: vertical line, horizontal line, single point */
	if (x0 == x1) {
		if (y0 == y1) {
			fill_rect_{{ ps.suffix }}(pixmap, x0-r, y0-r, x0+r, y0+r, pixval);
			return;
		}
		if (y0 > y1)
			GP_SWAP(y0, y1);

		fill_rect_{{ ps.suffix }}(pixmap, x0-r, y0, x0+r, y1, pixval);
		return;
	}
	if (y0 == y1) {
		fill_rect_{{ ps.suffix }}(pixmap, x0, y0-r, x1, y0+r, pixval);
		return;
	}

	if ((y1 - y0) / (x1 - x0))
		line_dy_{{ ps.suffix }}(pixmap, x0, y0, x1, y1, r, pixval);
	else
		line_dx_{{ ps.suffix }}(pixmap, x0, y0, x1, y1, r, pixval);
}

@ end

void gp_line_th_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                    gp_coord x1, gp_coord y1, gp_size r,
                    gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(gp_line_th_raw, pixmap, pixmap, x0, y0, x1, y1, r, pixel);
}

void gp_line_th(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                gp_coord x1, gp_coord y1, gp_size r,
                gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);

	gp_line_th_raw(pixmap, x0, y0, x1, y1, r, pixel);
}
