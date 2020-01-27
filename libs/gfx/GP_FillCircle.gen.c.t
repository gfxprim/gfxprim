@ include source.t
/*
 * A filled circle drawing algorithm.
 *
 * Copyright (C) 2009-2012 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_transform.h>
#include <core/gp_fn_per_bpp.h>
#include "gfx/GP_HLine.h"
#include "gfx/GP_Circle.h"
#include "gfx/GP_CircleSeg.h"

/*
 * A filled circle drawing algorithm.
 *
 * A filled circle is drawn in the same way as an unfilled one,
 * in a top-down, line per line manner, except that we don't need to draw
 * four points in each X step. Instead, we just iterate X
 * until we accumulate enough Y changes to reach the next line,
 * and then draw the full line. The top and bottom half are mirrored.
 */
@ for ps in pixelsizes:

static void fill_circle_{{ ps.suffix }}(gp_pixmap *pixmap,
	gp_coord xcenter, gp_coord ycenter, gp_size r, gp_pixel pixel)
{
	/* for r == 0, circle degenerates to a point */
	if (r == 0) {
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, xcenter, ycenter, pixel);
		return;
	}

	int x, y, error;
	for (x = 0, error = -r, y = r; y >= 0; y--) {
		while (error < 0) {
			error += 2*x + 1;
			x++;
		}
		error += -2*y + 1;
		gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter+x-1, ycenter-y, pixel);
		gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter+x-1, ycenter+y, pixel);
	}
}

static void fill_circle_seg_{{ ps.suffix }}(gp_pixmap *pixmap,
	gp_coord xcenter, gp_coord ycenter, gp_size r, uint8_t seg_flag, gp_pixel pixel)
{
	/* for r == 0, circle degenerates to a point */
	if (r == 0) {
		gp_putpixel_raw_{{ ps.suffix }}(pixmap, xcenter, ycenter, pixel);
		return;
	}

	int x, y, error;
	for (x = 0, error = -r, y = r; y >= 0; y--) {
		while (error < 0) {
			error += 2*x + 1;
			x++;
		}
		error += -2*y + 1;

		if (seg_flag & GP_CIRCLE_SEG1)
			gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter, xcenter+x-1, ycenter-y, pixel);

		if (seg_flag & GP_CIRCLE_SEG2)
			gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter, ycenter-y, pixel);

		if (seg_flag & GP_CIRCLE_SEG3)
			gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter, ycenter+y, pixel);

		if (seg_flag & GP_CIRCLE_SEG4)
			gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter, xcenter+x-1, ycenter+y, pixel);
	}
}

@ end

void gp_fill_circle_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                        gp_size r, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(fill_circle, pixmap, pixmap,
	                     xcenter, ycenter, r, pixel);
}

void gp_fill_circle_seg_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                            gp_size r, uint8_t seg_flag, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(fill_circle_seg, pixmap, pixmap,
	                     xcenter, ycenter, r, seg_flag, pixel);
}

void gp_fill_circle(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                    gp_size r, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);

	gp_fill_circle_raw(pixmap, xcenter, ycenter, r, pixel);
}
