@ include source.t
/*
 * Vertical Line drawing algorithm.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_GetPutPixel.h"
#include "gfx/GP_VLine.gen.h"

@ for ps in pixelsizes:
void GP_VLine_Raw_{{ ps.suffix }}(GP_Context *context, GP_Coord x,
			GP_Coord y0, GP_Coord y1, GP_Pixel pixel)
{
	int y;

	for (y = y0; y <= y1; y++)
		GP_PutPixel_Raw_{{ ps.suffix }}(context, x, y, pixel);
}

