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
 *                                                                           *
 *****************************************************************************/

#include "GP_Common.h"
#include "GP_Convert.h"
#include "GP_GetPutPixel.h"
#include "GP_TestingCore.h"
#include "GP_TestingRandom.h"

/*
 * TODO: Use a different version for palette types.
 */
GP_Pixel GP_RandomColor(GP_PixelType type)
{
	return GP_RGBAToPixel(GP_TestingRandom() % 256, GP_TestingRandom() % 256, 
	                      GP_TestingRandom() % 256, GP_TestingRandom() % 256, type);
}

void GP_RandomizeRect(GP_Context *context, GP_Coord x, GP_Coord y, GP_Size w, GP_Size h)
{
	GP_CHECK(context);
	for (GP_Size i = 0; i < w; i++)
		for (GP_Size j = 0; j < h; j++)
			GP_PutPixel(context, i + x, j + y, GP_RandomColor(context->pixel_type));
}

int GP_EqualColors(GP_Pixel p1, GP_PixelType t1, GP_Pixel p2, GP_PixelType t2)
{
	GP_Pixel col1 = GP_PixelToRGBA8888(p1, t1);
	GP_Pixel col2 = GP_PixelToRGBA8888(p2, t2);
	return (col1 & 0xffffffff) == (col2 & 0xffffffff);
}

int GP_EqualRects(const GP_Context *c1, GP_Coord x1, GP_Coord y1, GP_Size w, GP_Size h,
                  const GP_Context *c2, GP_Coord x2, GP_Coord y2)
{
	GP_CHECK(c1);
	GP_CHECK(c2);
	for (GP_Size i = 0; i < w; i++)
		for (GP_Size j = 0; j < h; j++) {
			if (!GP_EqualColors(GP_GetPixel(c1, x1 + i, y1 + j), c1->pixel_type,
			                    GP_GetPixel(c2, x2 + i, y2 + j), c2->pixel_type))
				return 0;
		}
	return 1;
}
