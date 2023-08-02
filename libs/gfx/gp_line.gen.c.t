@ include source.t
/*
 * Line drawing algorithm.
 *
 * Copyright (C) 2009-2012 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2018 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_common.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_pixel_pack.gen.h>

#include <gfx/gp_vline.h>
#include <gfx/gp_hline.h>
#include <gfx/gp_line.h>
#include <gfx/gp_line_clip.h>

/*
 * The classical Bresenham line drawing algorithm.
 * Please see http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * for a nice and understandable description.
 */

@ for ps in pixelpacks:
static void line_dy_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int y0, int x1, int y1, gp_pixel pixval)
{
	if (y0 > y1) {
		GP_SWAP(y0, y1);
		GP_SWAP(x0, x1);
	}

	int deltay = y1 - y0;
	int deltax = GP_ABS(x1 - x0);

	int error = deltay / 2;

	int x = 0, y;
	int xstep = (x0 < x1) ? 1 : -1;

	for (y = 0; y <= deltay/2; y++) {
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x0+x, y0+y, pixval);
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x1-x, y1-y, pixval);

		error -= deltax;
		if (error < 0) {
			x += xstep;
			error += deltay;
		}
	}
}

static void line_dx_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int y0, int x1, int y1, gp_pixel pixval)
{
	if (x0 > x1) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	int deltax = x1 - x0;
	int deltay = GP_ABS(y1 - y0);

	int error = deltax/2;

	int y = 0, x;
	int ystep = (y0 < y1) ? 1 : -1;

	for (x = 0; x <= deltax/2; x++) {
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x0+x, y0+y, pixval);
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, x1-x, y1-y, pixval);

		error -= deltay;
		if (error < 0) {
			y += ystep;
			error += deltax;
		}
	}
}

void gp_line_raw_{{ ps.suffix }}(gp_pixmap *pixmap, int x0, int y0,
	int x1, int y1, gp_pixel pixval)
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
			gp_putpixel_raw_clipped_{{ ps.suffix }}(pixmap,
					x0, y0, pixval);
			return;
		}
		if (y0 > y1)
			GP_SWAP(y0, y1);
		gp_vline_raw_{{ ps.suffix }}(pixmap, x0, y0, y1, pixval);
		return;
	}
	if (y0 == y1) {
		gp_hline_raw_{{ ps.suffix }}(pixmap, x0, x1, y0, pixval);
		return;
	}

	/*
	 * Which axis is longer? Swap the coordinates if necessary so
	 * that the X axis is always the longer one and Y is shorter.
	 */
	if ((y1 - y0) / (x1 - x0))
		line_dy_{{ ps.suffix }}(pixmap, x0, y0, x1, y1, pixval);
	else
		line_dx_{{ ps.suffix }}(pixmap, x0, y0, x1, y1, pixval);
}

@ end

void gp_line_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                 gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_PACK_PIXMAP(gp_line_raw, pixmap, pixmap, x0, y0, x1, y1,
	                      pixel);
}

void gp_line(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
             gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);

	gp_line_raw(pixmap, x0, y0, x1, y1, pixel);
}
