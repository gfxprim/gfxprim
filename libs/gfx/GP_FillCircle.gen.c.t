@ include source.t
/*
 * A filled circle drawing algorithm.
 *
 * Copyright (C) 2009-2012 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_GetPutPixel.h"
#include "core/GP_Transform.h"
#include "core/GP_FnPerBpp.h"
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

static void GP_FillCircle_Raw_{{ ps.suffix }}(GP_Pixmap *pixmap,
	GP_Coord xcenter, GP_Coord ycenter, GP_Size r, GP_Pixel pixel)
{
	/* for r == 0, circle degenerates to a point */
	if (r == 0) {
		GP_PutPixel_Raw_{{ ps.suffix }}(pixmap, xcenter, ycenter, pixel);
		return;
	}

	int x, y, error;
	for (x = 0, error = -r, y = r; y >= 0; y--) {
		while (error < 0) {
			error += 2*x + 1;
			x++;
		}
		error += -2*y + 1;
		GP_HLine_Raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter+x-1, ycenter-y, pixel);
		GP_HLine_Raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter+x-1, ycenter+y, pixel);
	}
}

static void GP_FillCircleSeg_Raw_{{ ps.suffix }}(GP_Pixmap *pixmap,
	GP_Coord xcenter, GP_Coord ycenter, GP_Size r, uint8_t seg_flag, GP_Pixel pixel)
{
	/* for r == 0, circle degenerates to a point */
	if (r == 0) {
		GP_PutPixel_Raw_{{ ps.suffix }}(pixmap, xcenter, ycenter, pixel);
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
			GP_HLine_Raw_{{ ps.suffix }}(pixmap, xcenter, xcenter+x-1, ycenter-y, pixel);

		if (seg_flag & GP_CIRCLE_SEG2)
			GP_HLine_Raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter, ycenter-y, pixel);

		if (seg_flag & GP_CIRCLE_SEG3)
			GP_HLine_Raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter, ycenter+y, pixel);

		if (seg_flag & GP_CIRCLE_SEG4)
			GP_HLine_Raw_{{ ps.suffix }}(pixmap, xcenter, xcenter+x-1, ycenter+y, pixel);
	}
}

@ end

void GP_FillCircle_Raw(GP_Pixmap *pixmap, GP_Coord xcenter, GP_Coord ycenter,
                       GP_Size r, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(GP_FillCircle_Raw, pixmap, pixmap,
	                      xcenter, ycenter, r, pixel);
}

void GP_FillCircleSeg_Raw(GP_Pixmap *pixmap, GP_Coord xcenter, GP_Coord ycenter,
                          GP_Size r, uint8_t seg_flag, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(GP_FillCircleSeg_Raw, pixmap, pixmap,
	                      xcenter, ycenter, r, seg_flag, pixel);
}

void GP_FillCircle(GP_Pixmap *pixmap, GP_Coord xcenter, GP_Coord ycenter,
                   GP_Size r, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);

	GP_FillCircle_Raw(pixmap, xcenter, ycenter, r, pixel);
}
