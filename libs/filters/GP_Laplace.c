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

int GP_FilterLaplace(const GP_Pixmap *src, GP_Pixmap *dst,
		     GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Laplace filter %ux%u", src->w, src->h);

	float kern[9] = {0,  1,  0,
	                 1, -4,  1,
	                 0,  1,  0};

	if (GP_FilterLinearConvolution_Raw(src, 0, 0, src->w, src->h,
	                                   dst, 0, 0, kern, 3, 3, 1, callback))
		return 1;

	return 0;
}

GP_Pixmap *GP_FilterLaplaceAlloc(const GP_Pixmap *src,
                                  GP_ProgressCallback *callback)
{
	GP_Pixmap *ret = GP_PixmapCopy(src, 0);

	if (ret == NULL)
		return NULL;

	if (GP_FilterLaplace(src, ret, callback)) {
		GP_PixmapFree(ret);
		return NULL;
	}

	return ret;
}


int GP_FilterEdgeSharpening(const GP_Pixmap *src, GP_Pixmap *dst,
                            float w, GP_ProgressCallback *callback)
{
	/* Identity kernel */
	float kern[9] = {0,  0,  0,
	                 0,  1,  0,
	                 0,  0,  0};

	GP_DEBUG(1, "Laplace Edge Sharpening filter %ux%u w=%f",
	         src->w, src->h, w);

	/* Create combined kernel */
	kern[1] -=  1.00 * w;
	kern[3] -=  1.00 * w;
	kern[4] -= -4.00 * w;
	kern[5] -=  1.00 * w;
	kern[7] -=  1.00 * w;

	if (GP_FilterLinearConvolution_Raw(src, 0, 0, src->w, src->h,
	                                   dst, 0, 0,  kern, 3, 3, 1, callback))
		return 1;

	return 0;
}

GP_Pixmap *GP_FilterEdgeSharpeningAlloc(const GP_Pixmap *src, float w,
                                         GP_ProgressCallback *callback)
{
	GP_Pixmap *ret = GP_PixmapCopy(src, 0);

	if (ret == NULL)
		return NULL;

	if (GP_FilterEdgeSharpening(src, ret, w, callback)) {
		GP_PixmapFree(ret);
		return NULL;
	}

	return ret;
}
