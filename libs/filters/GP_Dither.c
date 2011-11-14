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

#include "core/GP_Core.h"
#include "core/GP_FnPerBpp.h"
#include "core/GP_Pixel.h"

#include "GP_Dither.h"

#include <string.h>

/*
 * Experimental Floyd Steinberg XXX -> G1
 */
int GP_FilterFloydSteinberg_to_G1_Raw(const GP_Context *src, GP_Context *dst,
                                      GP_ProgressCallback *callback)
{
	float errors[2][src->w];
	
	GP_DEBUG(1, "Floyd Steinberg %s to %s size %ux%u",
	            GP_PixelTypeName(src->pixel_type),
		    GP_PixelTypeName(GP_PIXEL_G1), src->w, src->h);

	GP_Coord x, y;

	memset(errors[0], 0, src->w * sizeof(float));
	memset(errors[1], 0, src->w * sizeof(float));

	for (y = 0; y < (GP_Coord)src->h; y++) {
		for (x = 0; x < (GP_Coord)src->w; x++) { 
			GP_Pixel pix = GP_GetPixel(src, x, y);
			float val = GP_ConvertPixel(pix, src->pixel_type,
			                            GP_PIXEL_G8);

			val += errors[y%2][x];
			
			float err;

			if (val > 127) {
				err = val - 255;
				GP_PutPixel_Raw_1BPP_LE(dst, x, y, 1);
			} else {
				err = val - 0;
				GP_PutPixel_Raw_1BPP_LE(dst, x, y, 0);
			}

			if (x + 1 < (GP_Coord)src->w)
				errors[y%2][x+1] += 7 * err / 16;

			if (x > 1)
				errors[!(y%2)][x-1] += 3 * err / 16;
		
			errors[!(y%2)][x] += 5 * err / 16;

			if (x + 1 < (GP_Coord)src->w)
				errors[!(y%2)][x+1] += err / 16;
		}

		memset(errors[y%2], 0, src->w * sizeof(float));
		
		if (GP_ProgressCallbackReport(callback, y, src->h, src->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

GP_Context *GP_FilterFloydSteinberg_to_G1(const GP_Context *src,
                                          GP_Context *dst,
                                          GP_ProgressCallback *callback)
{
	GP_Context *ret = dst;

	if (ret == NULL) {
		ret = GP_ContextAlloc(src->w, src->h, GP_PIXEL_G1);
	
		if (ret == NULL) {
			GP_DEBUG(1, "Malloc failed :(");
			return NULL;
		}
	}

	if (GP_FilterFloydSteinberg_to_G1_Raw(src, ret, callback)) {
		if (dst == NULL)
			free(ret);
		return NULL;
	}

	return ret;
}

GP_Context *GP_FilterFloydSteinberg_from_RGB888(const GP_Context *src,
                                                GP_Context *dst,
                                                GP_PixelType pixel_type,
                                                GP_ProgressCallback *callback)
{
	GP_Context *ret = dst;

	if (ret == NULL) {
		ret = GP_ContextAlloc(src->w, src->h, pixel_type);
	
		if (ret == NULL) {
			GP_DEBUG(1, "Malloc failed :(");
			return NULL;
		}
	}

	if (GP_FilterFloydSteinberg_RGB888_to_XXX_Raw(src, ret, callback)) {
		if (dst == NULL)
			free(ret);
		return NULL;
	}

	return ret;
}
