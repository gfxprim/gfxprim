@ include source.t
/*
 * Filled triangle drawing algorithm.
 *
 * Copyright (C) 2009-2018 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/gp_common.h"
#include <core/gp_get_put_pixel.h>
#include <core/gp_fn_per_bpp.h>
#include <gfx/GP_HLine.h>

typedef struct gp_line {
	gp_coord x, dx, dy, error, xstep;
} gp_line;

static void gp_line_init(gp_line *self, gp_coord x0, gp_coord y0,
                         gp_coord x1, gp_coord y1)
{
	if (y0 > y1) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	self->x  = 0;
	self->dy = y1 - y0;
	self->dx = GP_ABS(x1 - x0);
	self->error = self->dx/2;
	self->xstep = (x0 < x1) ? 1 : -1;
}

static void gp_line_nexty(gp_line *self)
{
	self->error -= self->dx;

	while (self->error < 0) {
		self->x += self->xstep;
                self->error += self->dy;
        }
}

@ for ps in pixelsizes:
static void fill_triangle_{{ ps.suffix }}(gp_pixmap *pixmap,
                                          gp_coord x0, gp_coord y0,
                                          gp_coord x1, gp_coord y1,
                                          gp_coord x2, gp_coord y2,
					  gp_pixel pixval)
{
	if (y0 > y1) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	if (y0 > y2) {
		GP_SWAP(x0, x2);
		GP_SWAP(y0, y2);
	}

	if (y1 > y2) {
		GP_SWAP(x1, x2);
		GP_SWAP(y1, y2);
	}

	if (y0 == y2) {
		gp_hline_raw_{{ ps.suffix }}(pixmap,
		                             GP_MIN3(x0, x1, x2),
		                             GP_MAX3(x0, x1, x2),
		                             y0, pixval);
		return;
	}

	gp_line l1, l2;
	gp_line_init(&l1, x0, y0, x1, y1);
	gp_line_init(&l2, x0, y0, x2, y2);

	gp_coord y, deltay;

	deltay = y1 - y0;

	for (y = 0; y < deltay; y++) {
		gp_hline_raw_{{ ps.suffix }}(pixmap, x0 + l1.x, x0 + l2.x, y0 + y, pixval);
		gp_line_nexty(&l1);
		gp_line_nexty(&l2);
	}

	deltay = y2 - y1;

	gp_line_init(&l1, x1, y1, x2, y2);

	for (y = 0; y < deltay; y++) {
		gp_hline_raw_{{ ps.suffix }}(pixmap, x1 + l1.x, x0 + l2.x, y1 + y, pixval);
		gp_line_nexty(&l1);
		gp_line_nexty(&l2);
	}

	gp_hline_raw_{{ ps.suffix }}(pixmap, x1 + l1.x, x0 + l2.x, y1 + y, pixval);
}

@ end

void gp_fill_triangle_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                          gp_coord x1, gp_coord y1, gp_coord x2, gp_coord y2, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(fill_triangle, pixmap, pixmap, x0, y0, x1, y1, x2, y2,
	                     pixel);
}
