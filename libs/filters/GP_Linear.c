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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <GP_Context.h>
#include <GP_GetPutPixel.h>

#include <GP_Debug.h>

#include "GP_Linear.h"

GP_Context *GP_FilterBrightness(const GP_Context *src, int32_t inc)
{
	GP_Context *res;

	GP_DEBUG(1, "Running Brightness filter with inc=%i", (int)inc);
	
	res = GP_ContextCopy(src, 0);

	if (res == NULL)
		return NULL;

	GP_FilterBrightness_Raw(src, res, inc);

	return res;
}

GP_Context *GP_FilterContrast(const GP_Context *src, float mul)
{
	GP_Context *res;

	GP_DEBUG(1, "Running Contrast filter with mul=%2.2f", mul);
	
	res = GP_ContextCopy(src, 0);

	if (res == NULL)
		return NULL;

	GP_FilterContrast_Raw(src, res, mul);

	return res;
}
