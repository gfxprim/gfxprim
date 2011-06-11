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

#include "GP_GetPutPixel.h"
#include "GP_Transform.h"

GP_Pixel GP_GetPixel(GP_Context *context, int x, int y)
{
	GP_TRANSFORM_POINT(context, x, y);
	if (GP_PIXEL_IS_CLIPPED(context, x, y)) 
		return 0;
	return GP_GetPixel_Raw(context, x, y);
}

void GP_PutPixel(GP_Context *context, int x, int y, GP_Pixel p)
{
	GP_TRANSFORM_POINT(context, x, y);
	if (! GP_PIXEL_IS_CLIPPED(context, x, y)) 
		GP_PutPixel_Raw(context, x, y, p);
}
