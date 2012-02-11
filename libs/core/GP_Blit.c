/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 * Copyright (C) 2011,2012 Cyril Hrubis    <metan@ucw.cz>                    *
 *                                                                           *
 *****************************************************************************/

#include "GP_Pixel.h"
#include "GP_GetPutPixel.h"
#include "GP_Context.h"
#include "GP_Convert.h"
#include "GP_Blit.h"
#include "GP_FnPerBpp.h"

/* Generated function */
void GP_BlitXYXY_Raw_Fast(const GP_Context *src,
                          GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
			  GP_Context *dst, GP_Coord x2, GP_Coord y2);


void GP_BlitXYXY_Naive(const GP_Context *src,
                       GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
                       GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
	GP_Coord x, y;

	for (y = y0; y <= y1; y++)
		for (x = x0; x <= x1; x++) {
			GP_Pixel p = GP_GetPixel(src, x, y);

			if (src->pixel_type != dst->pixel_type) 
				p = GP_ConvertContextPixel(p, src, dst);

			GP_PutPixel(dst, x2 + (x - x0), y2 + (y - y0), p);
		}
}

void GP_BlitXYXY(const GP_Context *src,
                 GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
                 GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
	/* Normalize source rectangle */
	if (x1 < x0)
		GP_SWAP(x0, x1);

	if (y1 < y0)
		GP_SWAP(y0, y1);

	/* All coordinates are inside of src the context */
	GP_CHECK(x0 < (GP_Coord)GP_ContextW(src));
	GP_CHECK(y0 < (GP_Coord)GP_ContextH(src));
	GP_CHECK(x1 < (GP_Coord)GP_ContextW(src));
	GP_CHECK(y1 < (GP_Coord)GP_ContextH(src));

	/* Destination is big enough */
	GP_CHECK(x2 + (x1 - x0) < (GP_Coord)GP_ContextW(dst));
	GP_CHECK(y2 + (y1 - y0) < (GP_Coord)GP_ContextH(dst));

	if (GP_CONTEXT_ROTATION_EQUAL(src, dst)) {
		GP_BlitXYXY_Raw_Fast(src, x0, y0, x1, y1, dst, x2, y2);
		return;
	}
	
	GP_BlitXYXY_Naive(src, x0, y0, x1, y1, dst, x2, y2);
}

void GP_BlitXYWH(const GP_Context *src,
                 GP_Coord x0, GP_Coord y0, GP_Size w0, GP_Size h0,
                 GP_Context *dst, GP_Coord x1, GP_Coord y1)
{
	if (w0 == 0 || h0 == 0)
		return;

	GP_BlitXYXY(src, x0, y0, x0 + w0 - 1, y0 + h0 - 1, dst, x1, y1);
}

void GP_BlitXYXY_Raw(const GP_Context *src,
                     GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
                     GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
	/* Normalize source rectangle */
	if (x1 < x0)
		GP_SWAP(x0, x1);

	if (y1 < y0)
		GP_SWAP(y0, y1);

	/* All coordinates are inside of src the context */
	GP_CHECK(x0 < (GP_Coord)src->w);
	GP_CHECK(y0 < (GP_Coord)src->h);
	GP_CHECK(x1 < (GP_Coord)src->w);
	GP_CHECK(y1 < (GP_Coord)src->h);

	/* Destination is big enough */
	GP_CHECK(x2 + (x1 - x0) < (GP_Coord)dst->w);
	GP_CHECK(y2 + (y1 - y0) < (GP_Coord)dst->h);

	GP_BlitXYXY_Raw_Fast(src, x0, y0, x1, y1, dst, x2, y2);
}

void GP_BlitXYWH_Raw(const GP_Context *src,
                     GP_Coord x0, GP_Coord y0, GP_Size w0, GP_Size h0,
                     GP_Context *dst, GP_Coord x2, GP_Coord y2)
{
	if (w0 == 0 || h0 == 0)
		return;

	GP_BlitXYXY_Raw(src, x0, y0, x0 + w0 - 1, y0 + h0 - 1, dst, x2, y2);
}
