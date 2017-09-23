@ include source.t
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_GetPutPixel.h"
#include "core/GP_FnPerBpp.h"
#include "gfx/GP_HLine.h"
#include "gfx/GP_VLine.h"
#include "gfx/GP_Ellipse.h"

/*
 * A filled ellipse drawing algorithm.
 *
 * The algorithm is exactly the same as with GP_Ellipse() except that
 * we draw a line between each two points at each side of the X axis;
 * therefore, we don't need to draw any points during iterations of X,
 * we just iterate X until Y reaches next line, and then draw the full line.
 */

@ for ps in pixelsizes:

static void GP_FillEllipse_Raw_{{ ps.suffix }}(GP_Pixmap *pixmap, GP_Coord xcenter,
		GP_Coord ycenter, GP_Size a, GP_Size b, GP_Pixel pixel)
{
	/* Precompute quadratic terms. */
	int a2 = a*a;
	int b2 = b*b;

	/* Handle special case */
	if (a == 0) {
		GP_VLine_Raw_{{ ps.suffix }}(pixmap, xcenter, ycenter - b, ycenter + b, pixel);
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
		GP_HLine_Raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter+x-1, ycenter-y, pixel);
		GP_HLine_Raw_{{ ps.suffix }}(pixmap, xcenter-x+1, xcenter+x-1, ycenter+y, pixel);
	}
}

@ end

void GP_FillEllipse_Raw(GP_Pixmap *pixmap, GP_Coord xcenter, GP_Coord ycenter,
	                GP_Size a, GP_Size b, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(GP_FillEllipse_Raw, pixmap, pixmap,
	                      xcenter, ycenter, a, b, pixel);
}

void GP_FillEllipse(GP_Pixmap *pixmap, GP_Coord xcenter, GP_Coord ycenter,
                    GP_Size a, GP_Size b, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);
	GP_TRANSFORM_SWAP(pixmap, a, b);

	GP_FillEllipse_Raw(pixmap, xcenter, ycenter, a, b, pixel);
}
