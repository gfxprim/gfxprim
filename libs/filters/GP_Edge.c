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

#include "filters/GP_Linear.h"
#include "filters/GP_EdgeDetection.h"

/*
 * Apply prewitt operator.
 */
static int prewitt(const gp_pixmap *src, gp_pixmap *dx, gp_pixmap *dy,
                   gp_progress_cb *callback)
{
	float smooth_kern[3] = {1, 1, 1,};
	float grad_kern[3] = {-1, 0, 1};

	if (gp_filter_vhlinear_convolution_raw(src, 0, 0, src->w, src->h,
	                                     dx, 0, 0,
	                                     smooth_kern, 3, 1,
					     grad_kern, 3, 1, callback))
		return 1;

	if (gp_filter_vhlinear_convolution_raw(src, 0, 0, src->w, src->h,
	                                     dy, 0, 0,
	                                     grad_kern, 3, 1,
					     smooth_kern, 3, 1, callback))
		return 1;

	return 0;
}

/*
 * Apply sobel operator.
 */
static int sobel(const gp_pixmap *src, gp_pixmap *dx, gp_pixmap *dy,
                   gp_progress_cb *callback)
{
	float dx_kern[] = {
		-1,  0,  1,
		-2,  0,  2,
		-1,  0,  1,
	};

	gp_convolution_params dx_conv = {
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

	if (gp_filter_convolution_raw(&dx_conv))
		return 1;

	float dy_kern[] = {
		-1, -2, -1,
		 0,  0,  0,
		 1,  2,  1,
	};

	gp_convolution_params dy_conv = {
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

	if (gp_filter_convolution_raw(&dy_conv))
		return 1;

	return 0;
}

static int edge_detect(const gp_pixmap *src,
                       gp_pixmap **E, gp_pixmap **Phi, int type,
		       gp_progress_cb *callback)
{
	//TODO
	GP_ASSERT(src->pixel_type == GP_PIXEL_RGB888);

	gp_pixmap *dx, *dy;

	dx = gp_pixmap_copy(src, 0);
	dy = gp_pixmap_copy(src, 0);

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
			gp_pixel pix_x = gp_getpixel_raw_24BPP(dx, i, j);
			gp_pixel pix_y = gp_getpixel_raw_24BPP(dy, i, j);
			int Rx, Gx, Bx;
			int Ry, Gy, By;
			int RE, GE, BE;
			int RPhi, GPhi, BPhi;

			Rx = GP_PIXEL_GET_R_RGB888(pix_x);
			Gx = GP_PIXEL_GET_G_RGB888(pix_x);
			Bx = GP_PIXEL_GET_B_RGB888(pix_x);

			Ry = GP_PIXEL_GET_R_RGB888(pix_y);
			Gy = GP_PIXEL_GET_G_RGB888(pix_y);
			By = GP_PIXEL_GET_B_RGB888(pix_y);

			RE = sqrt(Rx*Rx + Ry*Ry) + 0.5;
			GE = sqrt(Gx*Gx + Gy*Gy) + 0.5;
			BE = sqrt(Bx*Bx + By*By) + 0.5;

			gp_putpixel_raw_24BPP(dx, i, j,
			                      GP_PIXEL_CREATE_RGB888(RE, GE, BE));

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

			gp_putpixel_raw_24BPP(dy, i, j,
			                      GP_PIXEL_CREATE_RGB888(RPhi, GPhi, BPhi));
		}
	}

	if (Phi != NULL)
		*Phi = dy;
	else
		gp_pixmap_free(dy);

	if (E != NULL)
		*E = dx;
	else
		gp_pixmap_free(dx);

	return 0;
err0:
	gp_pixmap_free(dx);
	gp_pixmap_free(dy);
	return 1;
}

int gp_filter_edge_sobel(const gp_pixmap *src,
                         gp_pixmap **E, gp_pixmap **Phi,
                         gp_progress_cb *callback)
{
	GP_DEBUG(1, "Sobel edge detection image %ux%u", src->w, src->h);

	return edge_detect(src, E, Phi, 0, callback);
}

int gp_filter_edge_prewitt(const gp_pixmap *src,
                           gp_pixmap **E, gp_pixmap **Phi,
                           gp_progress_cb *callback)
{
	GP_DEBUG(1, "Prewitt edge detection image %ux%u", src->w, src->h);

	return edge_detect(src, E, Phi, 1, callback);
}
