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

#include <math.h>

#include "core/GP_Debug.h"
#include "core/GP_GetPutPixel.h"

#include "GP_Linear.h"

/*
 * Apply prewitt operator.
 */
static int prewitt(const GP_Context *src, GP_Context *dx, GP_Context *dy,
                   GP_ProgressCallback *callback)
{
	float smooth_kern[3] = {1, 1, 1,};
	float grad_kern[3] = {-1, 0, 1};

	if (GP_FilterVHLinearConvolution_Raw(src, 0, 0, src->w, src->h,
	                                     dx, 0, 0,
	                                     smooth_kern, 3, 1,
					     grad_kern, 3, 1, callback))
		return 1;

	if (GP_FilterVHLinearConvolution_Raw(src, 0, 0, src->w, src->h,
	                                     dy, 0, 0,
	                                     grad_kern, 3, 1,
					     smooth_kern, 3, 1, callback))
		return 1;
	
	return 0;
}

/*
 * Apply sobel operator.
 */
static int sobel(const GP_Context *src, GP_Context *dx, GP_Context *dy,
                   GP_ProgressCallback *callback)
{
	float dx_kern[] = {
		-1,  0,  1,
		-2,  0,  2,
		-1,  0,  1,
	};

	GP_ConvolutionParams dx_conv = {
		.src = src,
		.x_src = 0,
		.y_src = 0,
		.w_src = src->w,
		.h_src = src->h,
		.dst = dx,
		.x_dst = 0,
		.y_dst = 0,
		.kernel = dx_kern,
		.kw = 3,
		.kh = 3,
		.kern_div = 1,
		.callback = callback,
	};

	if (GP_FilterConvolution_Raw(&dx_conv))
		return 1;
	
	float dy_kern[] = {
		-1, -2, -1,
		 0,  0,  0,
		 1,  2,  1,
	};
	
	GP_ConvolutionParams dy_conv = {
		.src = src,
		.x_src = 0,
		.y_src = 0,
		.w_src = src->w,
		.h_src = src->h,
		.dst = dy,
		.x_dst = 0,
		.y_dst = 0,
		.kernel = dy_kern,
		.kw = 3,
		.kh = 3,
		.kern_div = 1,
		.callback = callback,
	};

	if (GP_FilterConvolution_Raw(&dy_conv))
		return 1;

	return 0;
}

static int edge_detect(const GP_Context *src,
                       GP_Context **E, GP_Context **Phi, int type,
		       GP_ProgressCallback *callback)
{
	//TODO
	GP_ASSERT(src->pixel_type == GP_PIXEL_RGB888);

	GP_Context *dx, *dy;

	dx = GP_ContextCopy(src, 0);
	dy = GP_ContextCopy(src, 0);

	if (dx == NULL || dy == NULL)
		goto err0;

	switch (type) {
	case 0:
		if (sobel(src, dx, dy, callback))
			goto err0;
	break;
	case 1:
		if (prewitt(src, dx, dy, callback))
			goto err0;
	break;
	default:
		goto err0;
	}
	
	uint32_t i, j;
	
	for (i = 0; i < src->w; i++) {
		for (j = 0; j < src->h; j++) {
			GP_Pixel pix_x = GP_GetPixel_Raw_24BPP(dx, i, j);
			GP_Pixel pix_y = GP_GetPixel_Raw_24BPP(dy, i, j);
			int Rx, Gx, Bx;
			int Ry, Gy, By;
			int RE, GE, BE;
			int RPhi, GPhi, BPhi;

			Rx = GP_Pixel_GET_R_RGB888(pix_x);
			Gx = GP_Pixel_GET_G_RGB888(pix_x);
			Bx = GP_Pixel_GET_B_RGB888(pix_x);
			
			Ry = GP_Pixel_GET_R_RGB888(pix_y);
			Gy = GP_Pixel_GET_G_RGB888(pix_y);
			By = GP_Pixel_GET_B_RGB888(pix_y);
		
			RE = sqrt(Rx*Rx + Ry*Ry) + 0.5;
			GE = sqrt(Gx*Gx + Gy*Gy) + 0.5;
			BE = sqrt(Bx*Bx + By*By) + 0.5;
			
			GP_PutPixel_Raw_24BPP(dx, i, j,
			                      GP_Pixel_CREATE_RGB888(RE, GE, BE));
			
			if (Rx != 0 && Ry != 0)
				RPhi = ((atan2(Rx, Ry) + M_PI) * 255)/(2*M_PI);
			else
				RPhi = 0;
			
			if (Gx != 0 && Gy != 0)
				GPhi = ((atan2(Gx, Gy) + M_PI) * 255)/(2*M_PI);
			else
				GPhi = 0;
			
			if (Bx != 0 && By != 0)
				BPhi = ((atan2(Bx, By) + M_PI) * 255)/(2*M_PI);
			else
				BPhi = 0;
			
			GP_PutPixel_Raw_24BPP(dy, i, j,
			                      GP_Pixel_CREATE_RGB888(RPhi, GPhi, BPhi));
		}
	}
	
	if (Phi != NULL)
		*Phi = dy;
	else
		GP_ContextFree(dy);
	
	if (E != NULL)
		*E = dx;
	else
		GP_ContextFree(dx);

	return 0;
err0:
	GP_ContextFree(dx);
	GP_ContextFree(dy);
	return 1;
}

int GP_FilterEdgeSobel(const GP_Context *src,
                       GP_Context **E, GP_Context **Phi,
                       GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Sobel edge detection image %ux%u", src->w, src->h);

	return edge_detect(src, E, Phi, 0, callback);
}

int GP_FilterEdgePrewitt(const GP_Context *src,
                         GP_Context **E, GP_Context **Phi,
                         GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Prewitt edge detection image %ux%u", src->w, src->h);

	return edge_detect(src, E, Phi, 1, callback);
}
