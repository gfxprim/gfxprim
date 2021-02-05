// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_pixmap.h>
#include <core/gp_transform.h>
#include <gfx/gp_symbol.h>

static enum gp_symbol_type transform_symbol(gp_pixmap *pixmap, enum gp_symbol_type stype)
{
	if (pixmap->axes_swap) {
		switch (stype) {
		case GP_TRIANGLE_UP:
			stype = GP_TRIANGLE_LEFT;
		break;
		case GP_TRIANGLE_DOWN:
			stype = GP_TRIANGLE_RIGHT;
		break;
		case GP_TRIANGLE_LEFT:
			stype = GP_TRIANGLE_UP;
		break;
		case GP_TRIANGLE_RIGHT:
			stype = GP_TRIANGLE_DOWN;
		break;
		}
	}

	if (pixmap->x_swap) {
		switch (stype) {
		case GP_TRIANGLE_LEFT:
			stype = GP_TRIANGLE_RIGHT;
		break;
		case GP_TRIANGLE_RIGHT:
			stype = GP_TRIANGLE_LEFT;
		break;
		default:
		break;
		}
	}

	if (pixmap->y_swap) {
		switch (stype) {
		case GP_TRIANGLE_UP:
			stype = GP_TRIANGLE_DOWN;
		break;
		case GP_TRIANGLE_DOWN:
			stype = GP_TRIANGLE_UP;
		break;
		default:
		break;
		}
	}

	return stype;
}

void gp_symbol(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
               gp_size rx, gp_size ry, enum gp_symbol_type stype, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);
	GP_TRANSFORM_SWAP(pixmap, rx, ry);

	gp_symbol_raw(pixmap, xcenter, ycenter, rx, ry,
	              transform_symbol(pixmap, stype), pixel);
}
