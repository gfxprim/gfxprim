// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/gp_common.h"
#include "core/gp_types.h"
#include <gfx/gp_line_clip.h>

int gp_line_clip(int *px0, int *py0, int *px1, int *py1, int xmax, int ymax)
{
	float x0 = (float) *px0;
	float y0 = (float) *py0;
	float x1 = (float) *px1;
	float y1 = (float) *py1;

	/* horizontal and vertical line are special cases */
	if (y0 == y1) {

		/* orient the line from left to right */
		if (x1 < x0) {
			GP_SWAP(x0, x1);
			GP_SWAP(y0, y1);
		}

		/* check if it is not completely outside */
		if (x1 < 0 || x0 > xmax || y0 < 0 || y0 > ymax)
			return 0;

		x0 = GP_MAX(x0, 0);
		x1 = GP_MIN(x1, xmax);
		goto give_result;
	}
	if (x0 == x1) {

		/* orient the line from top to down */
		if (x1 < x0) {
			GP_SWAP(x0, x1);
			GP_SWAP(y0, y1);
		}

		/* check if it is not completely outside */
		if (y1 < 0 || y0 > ymax || x0 < 0 || x0 > xmax)
			return 0;

		/* clip it to the valid range */
		y0 = GP_MAX(y0, 0);
		y1 = GP_MIN(y1, ymax);
		goto give_result;
	}

	/* orient the line from left to right */
	if (x1 < x0) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	if (x1 < 0 || x0 > xmax || (y0 < 0 && y1 < 0) || (y0 > ymax && y1 > ymax)) {

		/* the line lies completely outside the rectangle */
		return 0;
	}

	float dx = (float)(x1 - x0);
	float dy = (float)(y1 - y0);
	float dyx = dy/dx;
	float dxy = dx/dy;

	/* clip the line against the left and right side of the rectangle */
	if (x0 < 0) {
		y0 = y0 - x0*dyx;
		x0 = 0;
	}
	if (x1 > xmax) {
		x1 = xmax;
		y1 = y0 + (x1-x0)*dyx;
	}

	if (y0 < 0.0f) {
		x0 = x0 - y0*dxy;
		y0 = 0.0f;
	} else if (y0 > ymax) {
		x0 = x0 + (ymax-y0)*dxy;
		y0 = ymax;
	}

	if (y1 < 0.0f) {
		x1 = x1 - y1*dxy;
		y1 = 0.0f;
	}
	else if (y1 > ymax) {
		x1 = x1 - (y1 - ymax)*dxy;
		y1 = ymax;
	}

	if (x0 < 0 || x0 > xmax || x1 < 0 || x1 > xmax) {

		/* the line misses the clip rectangle around the corner */
		return 0;
	}

give_result:

	*px0 = (int) x0;
	*py0 = (int) y0;
	*px1 = (int) x1;
	*py1 = (int) y1;
	return 1;
}
