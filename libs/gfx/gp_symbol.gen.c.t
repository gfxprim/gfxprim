@ include source.t
/*
 * Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/gp_common.h"
#include <core/gp_get_put_pixel.h>
#include <core/gp_fn_per_bpp.h>
#include <gfx/gp_symbol.h>
#include <gfx/gp_hline.h>
#include <gfx/gp_vline.h>

@ for ps in pixelsizes:
static void triangle_up_{{ ps.suffix }}(gp_pixmap *pixmap,
                                        gp_coord xcenter, gp_coord ycenter,
                                        gp_size rx, gp_size ry,
                                        gp_pixel pixel)
{
	gp_coord y;

	for (y = 0; y <= 2 * (gp_coord)ry; y++) {
		gp_coord len = (y * rx) / (2  * ry);
		gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter + len, xcenter - len, ycenter + y - ry, pixel);
	}
}

static void triangle_down_{{ ps.suffix }}(gp_pixmap *pixmap,
                                          gp_coord xcenter, gp_coord ycenter,
                                          gp_size rx, gp_size ry,
                                          gp_pixel pixel)
{
	gp_coord y;

	for (y = 0; y <= 2 * (gp_coord)ry; y++) {
		gp_coord len = (y * rx) / (2  * ry);
		gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter + len, xcenter - len, ycenter - y + ry, pixel);
	}
}

static void triangle_left_{{ ps.suffix }}(gp_pixmap *pixmap,
                                          gp_coord xcenter, gp_coord ycenter,
                                          gp_size rx, gp_size ry,
                                          gp_pixel pixel)
{
	gp_coord x;

	for (x = 0; x <= 2 * (gp_coord)rx; x++) {
		gp_coord len = (x * ry) / (2 * rx);
		gp_vline_raw_{{ ps.suffix }}_clip(pixmap, xcenter + x - rx, ycenter - len, ycenter + len, pixel);
	}
}

static void triangle_right_{{ ps.suffix }}(gp_pixmap *pixmap,
                                           gp_coord xcenter, gp_coord ycenter,
                                           gp_size rx, gp_size ry,
                                           gp_pixel pixel)
{
	gp_coord x;

	for (x = 0; x <= 2 * (gp_coord)rx; x++) {
		gp_coord len = (x * ry) / (2 * rx);
		gp_vline_raw_{{ ps.suffix }}_clip(pixmap, xcenter - x + rx, ycenter - len, ycenter + len, pixel);
	}
}

@ end


void gp_symbol_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                   gp_size rx, gp_size ry, enum gp_symbol_type stype, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	switch (stype) {
	case GP_TRIANGLE_UP:
		GP_FN_PER_BPP_PIXMAP(triangle_up, pixmap, pixmap, xcenter, ycenter, rx, ry, pixel);
	break;
	case GP_TRIANGLE_DOWN:
		GP_FN_PER_BPP_PIXMAP(triangle_down, pixmap, pixmap, xcenter, ycenter, rx, ry, pixel);
	break;
	case GP_TRIANGLE_LEFT:
		GP_FN_PER_BPP_PIXMAP(triangle_left, pixmap, pixmap, xcenter, ycenter, rx, ry, pixel);
	break;
	case GP_TRIANGLE_RIGHT:
		GP_FN_PER_BPP_PIXMAP(triangle_right, pixmap, pixmap, xcenter, ycenter, rx, ry, pixel);
	break;
	}
}
