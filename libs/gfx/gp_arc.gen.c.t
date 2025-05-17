@ include source.t
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_transform.h>
#include <core/gp_pixel_pack.gen.h>

#include <gfx/gp_arc.h>

@ for ps in pixelpacks:
static void arc_segment_raw_{{ ps.suffix }}(gp_pixmap *pixmap,
	gp_coord xcenter, gp_coord ycenter,
	unsigned int a, unsigned int b, int direction,
	double start, double end, gp_pixel pixval)
{
	/* Precompute quadratic terms. */
	int a2 = a*a;
	int b2 = b*b;

	/* Compute minimum and maximum value of X from the angles. */
	int x1 = (int)(cos(start)*a);
	int x2 = (int)(cos(end)*a);
	int xmin = GP_MIN(x1, x2);
	int xmax = GP_MAX(x1, x2);

	int x, y, error;
	for (x = 0, error = -b2*a, y = b; y >= 0; y--) {
		while (error < 0) {

			/* Calculate error(x+1) from error(x). */
			error += 2*x*b2 + b2;
			x++;

			if (direction < 0) {
				if ((-x+1) >= xmin && (-x+1) <= xmax) {
					gp_putpixel_raw_clipped_{{ ps.suffix }}(pixmap, xcenter-x+1, ycenter-y, pixval);
				}
				if ((x-1) >= xmin && (x-1) <= xmax) {
					gp_putpixel_raw_clipped_{{ ps.suffix }}(pixmap, xcenter+x-1, ycenter-y, pixval);
				}
			}
			if (direction > 0) {
				if ((-x+1) >= xmin && (-x+1) <= xmax) {
					gp_putpixel_raw_clipped_{{ ps.suffix }}(pixmap, xcenter-x+1, ycenter+y, pixval);
				}
				if ((x-1) >= xmin && (x-1) <= xmax) {
					gp_putpixel_raw_clipped_{{ ps.suffix }}(pixmap, xcenter+x-1, ycenter+y, pixval);
				}
			}
		}

		/* Calculate error(y-1) from error(y). */
		error += -2*y*a2 + a2;

		if (direction < 0) {
			if ((-x+1) >= xmin && (-x+1) <= xmax) {
				gp_putpixel_raw_clipped_{{ ps.suffix }}(pixmap, xcenter-x+1, ycenter-y, pixval);
			}
			if ((x-1) >= xmin && (x-1) <= xmax) {
				gp_putpixel_raw_clipped_{{ ps.suffix }}(pixmap, xcenter+x-1, ycenter-y, pixval);
			}
		}
		if (direction > 0) {
			if ((-x+1) >= xmin && (-x+1) <= xmax) {
				gp_putpixel_raw_clipped_{{ ps.suffix }}(pixmap, xcenter-x+1, ycenter+y, pixval);
			}
			if ((x-1) >= xmin && (x-1) <= xmax) {
				gp_putpixel_raw_clipped_{{ ps.suffix }}(pixmap, xcenter+x-1, ycenter+y, pixval);
			}
		}
	}
}

@ end

void gp_arc_segment_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
		        gp_size a, gp_size b, int direction,
		        double start, double end,
		        gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_PACK_PIXMAP(arc_segment_raw, pixmap, pixmap,
	                      xcenter, ycenter, a, b, direction,
	                      start, end, pixel);
}

void gp_arc_segment(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
		    gp_size a, gp_size b, int direction,
		    double start, double end,
		    gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	/* recalculate center point and swap a and b when axes are swapped */
	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);
	GP_TRANSFORM_SWAP(pixmap, a, b);

	gp_arc_segment_raw(pixmap, xcenter, ycenter, a, b, direction,
			   start, end, pixel);
}
