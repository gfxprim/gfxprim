@ include source.t
/*
 * Line drawing algorithm.
 *
 * Copyright (C) 2009-2012 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_Common.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_FnPerBpp.h"

#include "gfx/GP_VLine.h"
#include "gfx/GP_HLine.h"
#include "gfx/GP_Line.h"
#include "gfx/GP_LineClip.h"

/*
 * The classical Bresenham line drawing algorithm.
 * Please see http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * for a nice and understandable description.
 */

@ for ps in pixelsizes:
void GP_Line_Raw_{{ ps.suffix }}(GP_Pixmap *pixmap, int x0, int y0,
	int x1, int y1, GP_Pixel pixval)
{
	if (!GP_LineClip(&x0, &y0, &x1, &y1, pixmap->w - 1, pixmap->h - 1))
		return;

	GP_ASSERT(x0 >= 0 && x0 <= (int) pixmap->w-1);
	GP_ASSERT(x1 >= 0 && x1 <= (int) pixmap->w-1);
	GP_ASSERT(y0 >= 0 && y0 <= (int) pixmap->h-1);
	GP_ASSERT(y1 >= 0 && y1 <= (int) pixmap->h-1);

	/* special cases: vertical line, horizontal line, single point */
	if (x0 == x1) {
		if (y0 == y1) {
			GP_PutPixel_Raw_Clipped_{{ ps.suffix }}(pixmap,
					x0, y0, pixval);
			return;
		}
		GP_VLine_Raw(pixmap, x0, y0, y1, pixval);
		return;
	}
	if (y0 == y1) {
		GP_HLine_Raw(pixmap, x0, x1, y0, pixval);
		return;
	}

	/*
	 * Which axis is longer? Swap the coordinates if necessary so
	 * that the X axis is always the longer one and Y is shorter.
	 */
	int steep = abs(y1 - y0) / abs(x1 - x0);
	if (steep) {
		GP_SWAP(x0, y0);
		GP_SWAP(x1, y1);
	}
	if (x0 > x1) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	/* iterate over the longer axis, calculate values on the shorter */
	int deltax = x1 - x0;
	int deltay = abs(y1 - y0);

	/*
	 * start with error of 0.5 (multiplied by deltax for integer-only math),
	 * this reflects the fact that ideally, the coordinate should be
	 * in the middle of the pixel
	 */
	int error = deltax / 2;

	int y = y0, x;
	int ystep = (y0 < y1) ? 1 : -1;
	for (x = x0; x <= x1; x++) {

		if (steep)
			GP_PutPixel_Raw_{{ ps.suffix }}(pixmap, y, x,
								pixval);
		else
			GP_PutPixel_Raw_{{ ps.suffix }}(pixmap, x, y,
								pixval);

		error -= deltay;
		if (error < 0) {
			y += ystep;	/* next step on the shorter axis */
			error += deltax;
		}
	}
}

@ end

void GP_Line_Raw(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                 GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(GP_Line_Raw, pixmap, pixmap, x0, y0, x1, y1,
	                      pixel);
}

void GP_Line(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
             GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);

	GP_Line_Raw(pixmap, x0, y0, x1, y1, pixel);
}
