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

#include "core/GP_Debug.h"

#include "GP_Linear.h"

#include "GP_Convolution.h"
int GP_FilterConvolutionEx(const GP_Context *src,
                           GP_Coord x_src, GP_Coord y_src,
                           GP_Size w_src, GP_Coord h_src,
                           GP_Context *dst,
                           GP_Coord x_dst, GP_Coord y_dst,
                           const GP_FilterKernel2D *kernel,
                           GP_ProgressCallback *callback)
{
	GP_CHECK(src->pixel_type == dst->pixel_type);
	
	/* Check that destination is large enough */
	GP_CHECK(x_dst + (GP_Coord)w_src <= (GP_Coord)dst->w);
	GP_CHECK(y_dst + (GP_Coord)h_src <= (GP_Coord)dst->h);

	/* The source pixel coordinates are clamped inside of the filter */

	GP_DEBUG(1, "Linear convolution kernel size %ux%u",
	         kernel->w, kernel->h);

	return GP_FilterLinearConvolution_Raw(src, x_src, y_src, w_src, h_src,
	                                      dst, x_dst, y_dst, kernel->kernel,
	                                      kernel->w, kernel->h, kernel->div,
					      callback);
}

GP_Context *GP_FilterConvolutionExAlloc(const GP_Context *src,
                                        GP_Coord x_src, GP_Coord y_src,
                                        GP_Size w_src, GP_Size h_src,
                                        const GP_FilterKernel2D *kernel,
                                        GP_ProgressCallback *callback)
{
	GP_Context *ret = GP_ContextAlloc(w_src, h_src, src->pixel_type);

	GP_DEBUG(1, "Linear convolution kernel size %ux%u",
	         kernel->w, kernel->h);

	if (ret == NULL)
		return NULL;

	if (GP_FilterLinearConvolution_Raw(src, x_src, y_src, w_src, h_src,
	                                   ret, 0, 0, kernel->kernel, kernel->w,
					   kernel->h, kernel->div, callback)) {
		GP_ContextFree(ret);
		return NULL;
	}

	return 0;
}
