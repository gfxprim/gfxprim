@ include source.t
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_transform.h>
#include <core/gp_fn_per_bpp.h>
#include <gfx/gp_circle.h>
#include <gfx/gp_circle_seg.h>

/*
 * Circle drawing algorithm.
 *
 * The circle is drawn in a top-down manner. We start from the top, and
 * at each line, we iterate X until we accumulate enough changes to Y
 * to pass to the next line. In each step, 4 pixels are drawn:
 * (X, Y), (-X, Y), (X, -Y) and (-X, -Y).
 *
 * The math:
 * From the circle equation, for every point applies:
 *
 * x^2 + y^2 = r^2      ->       x^2 + y^2 - r^2 = 0
 *
 * which has an exact solution for a non-integer x.
 * For an integer approximation, we want to find x
 * for which
 *
 * x^2 + y^2 - r^2 = error
 *
 * where error should be as close to 0 as possible.
 * We find the x by incrementing its value until
 * we cross the zero error boundary.
 *
 * Optimization:
 * Significant amount of multiplications can be
 * saved when calculating error by re-using previous
 * error values. For error(x+1) we have:
 *
 * error(x+1) = (x+1)^2 + y^2 - r^2
 *
 * which can be expanded to (expanding (x+1)^2):
 *
 * error(x+1) = x^2 + 2*x + 1 + y^2 - r^2
 *
 * and after substituting the error(x) we already know:
 *
 * error(x+1) = error(x) + 2*x + 1
 *
 * The same can be done for calculating
 * error(y-1) from error(y).
 */

@ def circle(ps, putpixel):
	int x, y, error;
	for (x = 0, error = -r, y = r; y >= 0; y--) {
		/* Iterate X until we can pass to the next line. */
		while (error < 0) {
			error += 2*x + 1;
			x++;
			{{ putpixel }}{{ ps.suffix }}(pixmap, xcenter-x+1, ycenter-y, pixel);
			{{ putpixel }}{{ ps.suffix }}(pixmap, xcenter+x-1, ycenter-y, pixel);
			{{ putpixel }}{{ ps.suffix }}(pixmap, xcenter-x+1, ycenter+y, pixel);
			{{ putpixel }}{{ ps.suffix }}(pixmap, xcenter+x-1, ycenter+y, pixel);
		}

		/* Enough changes accumulated, go to next line. */
		error += -2*y + 1;
		{{ putpixel }}{{ ps.suffix }}(pixmap, xcenter-x+1, ycenter-y, pixel);
		{{ putpixel }}{{ ps.suffix }}(pixmap, xcenter+x-1, ycenter-y, pixel);
		{{ putpixel }}{{ ps.suffix }}(pixmap, xcenter-x+1, ycenter+y, pixel);
		{{ putpixel }}{{ ps.suffix }}(pixmap, xcenter+x-1, ycenter+y, pixel);
	}
@ end
@ for ps in pixelsizes:
static void circle_{{ ps.suffix }}(gp_pixmap *pixmap,
	gp_coord xcenter, gp_coord ycenter, gp_size r, gp_pixel pixel)
{
	/* for r == 0, circle degenerates to a point */
	if (r == 0) {
		gp_putpixel_raw_clipped_{{ ps.suffix }}(pixmap, xcenter, ycenter, pixel);
		return;
	}

	if (xcenter < 0 || ycenter < 0 ||
	    (gp_size)xcenter < r || (gp_size)ycenter < r ||
	    xcenter + r >= pixmap->w || ycenter + r >= pixmap->h) {
	{@ circle(ps, "gp_putpixel_raw_clipped_") @}
	} else {
	{@ circle(ps, "gp_putpixel_raw_") @}
	}
}
@ end

void gp_circle_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                   gp_size r, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(circle, pixmap, pixmap,
	                     xcenter, ycenter, r, pixel);
}

void gp_circle(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
               gp_size r, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);

	gp_circle_raw(pixmap, xcenter, ycenter, r, pixel);
}
