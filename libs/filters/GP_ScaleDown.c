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

#include <GP_Resize.h>

GP_Context *GP_ScaleDown(GP_Context *src)
{
	uint32_t w, h, x, y;
	GP_Context *dst;

	if (src->pixel_type != GP_PIXEL_RGB888)
		return NULL;

	w = src->w/2;
	h = src->h/2;

	dst = GP_ContextAlloc(w, h, GP_PIXEL_RGB888);

	if (dst == NULL)
		return NULL;

	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, 2*x, 2*y);

			GP_PutPixel_Raw_24BPP(dst, x, y, pix);
		}

	return dst;
}
