@ include source.t
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_fn_per_bpp.h>
#include <gfx/gp_hline.h>
#include <gfx/gp_vline.h>
#include <gfx/gp_ellipse.h>

/*
 * A filled ellipse drawing algorithm.
 *
 * The algorithm is exactly the same as with gp_ellipse() except that
 * we draw a line between each two points at each side of the X axis;
 * therefore, we don't need to draw any points during iterations of X,
 * we just iterate X until Y reaches next line, and then draw the full line.
 */

@ for ps in pixelsizes:

static void gp_fill_ellipse_raw_{{ ps.suffix }}(gp_pixmap *pixmap, gp_coord xcenter,
		gp_coord ycenter, gp_size a, gp_size b, gp_pixel pixel)
{
	/* Precompute quadratic terms. */
	int a2 = a*a;
	int b2 = b*b;

	/* Handle special case */
	if (a == 0) {
		gp_vline_raw(pixmap, xcenter, ycenter - b, ycenter + b, pixel);
		return;
	}

	int x, y, error;
	for (x = 0, error = -b2*a, y = b; y >= 0; y--) {
		while (error < 0) {
			error += b2 * (2*x + 1);
			x++;
		}
		error += a2 * (-2*y + 1);

		/* Draw two horizontal lines reflected across Y. */
		gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter+x-1, ycenter-y, pixel);
		gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter+x-1, ycenter+y, pixel);
	}
}

@ end

void gp_fill_ellipse_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
	                 gp_size a, gp_size b, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(gp_fill_ellipse_raw, pixmap, pixmap,
	                     xcenter, ycenter, a, b, pixel);
}

void gp_fill_ellipse(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                    gp_size a, gp_size b, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);
	GP_TRANSFORM_SWAP(pixmap, a, b);

	gp_fill_ellipse_raw(pixmap, xcenter, ycenter, a, b, pixel);
}
