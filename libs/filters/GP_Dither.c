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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "GP_Dither.h"

int GP_FilterFloydSteinberg_RGB888(const GP_Context *src,
                                   GP_Context *dst,
                                   GP_ProgressCallback *callback)
{
	GP_CHECK(src->pixel_type == GP_PIXEL_RGB888);
	GP_CHECK(src->w <= dst->w);
	GP_CHECK(src->h <= dst->h);

	return GP_FilterFloydSteinberg_RGB888_Raw(src, dst, callback);
}


GP_Context *GP_FilterFloydSteinberg_RGB888_Alloc(const GP_Context *src,
                                                 GP_PixelType pixel_type,
                                                 GP_ProgressCallback *callback)
{
	GP_Context *ret;

	ret = GP_ContextAlloc(src->w, src->h, pixel_type);

	if (ret == NULL)
		return NULL;

	if (GP_FilterFloydSteinberg_RGB888_Raw(src, ret, callback)) {
		GP_ContextFree(ret);
		return NULL;
	}

	return ret;
}


int GP_FilterHilbertPeano_RGB888(const GP_Context *src,
                                 GP_Context *dst,
                                 GP_ProgressCallback *callback)
{
	GP_CHECK(src->pixel_type == GP_PIXEL_RGB888);
	GP_CHECK(src->w <= dst->w);
	GP_CHECK(src->h <= dst->h);

	return GP_FilterHilbertPeano_RGB888_Raw(src, dst, callback);
}

GP_Context *GP_FilterHilbertPeano_RGB888_Alloc(const GP_Context *src,
                                               GP_PixelType pixel_type,
                                               GP_ProgressCallback *callback)
{
	GP_Context *ret;

	ret = GP_ContextAlloc(src->w, src->h, pixel_type);

	if (ret == NULL)
		return NULL;

	if (GP_FilterHilbertPeano_RGB888_Raw(src, ret, callback)) {
		GP_ContextFree(ret);
		return NULL;
	}

	return ret;
}
