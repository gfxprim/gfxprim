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
#include "core/GP_GetPutPixel.h"

#include "GP_Linear.h"

#include "GP_Laplace.h"

int GP_FilterLaplace(const GP_Context *src, GP_Context *dst,
		     GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Laplace filter %ux%u", src->w, src->h);

	float kern[3] = {1, -2, 1};

	if (GP_FilterVHLinearConvolution_Raw(src, dst, kern, 3, 1,
	                                     kern, 3, 1, callback))
		return 1;

	return 0;
}

int GP_FilterEdgeSharpening(const GP_Context *src, GP_Context *dst,
                            float w, GP_ProgressCallback *callback)
{
	float kern[3] = {0, 1, 0};

	GP_DEBUG(1, "Laplace Edge Sharpening filter %ux%u w=%f",
	         src->w, src->h, w);

	kern[0] -=  1.00 * w;
	kern[1] -= -2.00 * w;
	kern[2] -=  1.00 * w;

	if (GP_FilterVHLinearConvolution_Raw(src, dst, kern, 3, 1,
	                                     kern, 3, 1, callback))
		return 1;

	return 0;
}
