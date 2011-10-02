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

void GP_FilterInterpolate_NN(GP_Context *src, GP_Context *res,
                             GP_ProgressCallback callback)
{
	GP_Coord x, y;
	
	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, res->w, res->h,
		    1.00 * res->w / src->w, 1.00 * res->h / src->h);

	for (y = 0; y < (GP_Coord)res->h; y++) {
		for (x = 0; x < (GP_Coord)res->w; x++) {
			GP_Coord xi = (1.00 * x / res->w) * src->w;
			GP_Coord yi = (1.00 * y / res->h) * src->h;
			
			GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, xi, yi);

			GP_PutPixel_Raw_24BPP(res, x, y, pix);
		}
	}
}

#define A 0.5

static float cubic(float x)
{
	if (x < 0)
		x = -x;

	if (x < 1)
		return (2 - A)*x*x*x + (A - 3)*x*x + 1;

	if (x < 2)
		return -A*x*x*x + 5*A*x*x - 8*A*x + 4*A;

	return 0;
}

typedef float v4sf __attribute__ ((vector_size (sizeof(float) * 4)));

typedef union v4f {
	v4sf v;
	float f[4];
} v4f;

#define MUL_V4SF(a, b) ((union v4f)((a).v * (b).v))
#define SUM_V4SF(a)    ((a).f[0] + (a).f[1] + (a).f[2] + (a).f[3])

void GP_FilterInterpolate_Cubic(GP_Context *src, GP_Context *res,
                                GP_ProgressCallback callback)
{
	float col_r[src->h], col_g[src->h], col_b[src->h];
	uint32_t i, j;

	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, res->w, res->h,
		    1.00 * res->w / src->w, 1.00 * res->h / src->h);

	for (i = 0; i < res->w; i++) {
		float x = (1.00 * i / res->w) * (src->w - 4.5) + 2.5;
		v4f cvx;
		int xi = x - 1;

		cvx.f[0] = cubic(x - xi);
		cvx.f[1] = cubic(x - xi - 1);
		cvx.f[2] = cubic(x - xi - 2);
		cvx.f[3] = cubic(x - xi - 3);
		
		if (xi < 0)
			xi = 0;

		if (xi > (int)src->w - 4)
			xi = src->w - 4;
	
		/* Generate interpolated column */
		for (j = 0; j < src->h; j++) {
			v4f rv, gv, bv;
			GP_Pixel pix[4];

			pix[0] = GP_GetPixel_Raw_24BPP(src, xi, j);
			pix[1] = GP_GetPixel_Raw_24BPP(src, xi + 1, j);
			pix[2] = GP_GetPixel_Raw_24BPP(src, xi + 2, j);
			pix[3] = GP_GetPixel_Raw_24BPP(src, xi + 3, j);
				
			rv.f[0] = GP_Pixel_GET_R_RGB888(pix[0]);
			rv.f[1] = GP_Pixel_GET_R_RGB888(pix[1]);
			rv.f[2] = GP_Pixel_GET_R_RGB888(pix[2]);
			rv.f[3] = GP_Pixel_GET_R_RGB888(pix[3]);
			
			gv.f[0] = GP_Pixel_GET_G_RGB888(pix[0]);
			gv.f[1] = GP_Pixel_GET_G_RGB888(pix[1]);
			gv.f[2] = GP_Pixel_GET_G_RGB888(pix[2]);
			gv.f[3] = GP_Pixel_GET_G_RGB888(pix[3]);
			
			bv.f[0] = GP_Pixel_GET_B_RGB888(pix[0]);
			bv.f[1] = GP_Pixel_GET_B_RGB888(pix[1]);
			bv.f[2] = GP_Pixel_GET_B_RGB888(pix[2]);
			bv.f[3] = GP_Pixel_GET_B_RGB888(pix[3]);

			rv = MUL_V4SF(rv, cvx);
			gv = MUL_V4SF(gv, cvx);
			bv = MUL_V4SF(bv, cvx);
			
			col_r[j] = SUM_V4SF(rv);
			col_g[j] = SUM_V4SF(gv);
			col_b[j] = SUM_V4SF(bv);
		}

		/* now interpolate column for new image */
		for (j = 0; j < res->h; j++) {
			float y = (1.00 * j / res->h) * (src->h - 4.5) + 2.5;
			v4f cvy, rv, gv, bv;
			float r, g, b;
			int yi = y - 1;
			
			cvy.f[0] = cubic(y - yi);
			cvy.f[1] = cubic(y - yi - 1);
			cvy.f[2] = cubic(y - yi - 2);
			cvy.f[3] = cubic(y - yi - 3);
				
			if (yi < 0)
				yi = 0;

			if (yi > (int)src->h - 4)
				yi = src->h - 4;
			
			rv.f[0] = col_r[yi];
			rv.f[1] = col_r[yi + 1];
			rv.f[2] = col_r[yi + 2];
			rv.f[3] = col_r[yi + 3];
			
			gv.f[0] = col_g[yi];
			gv.f[1] = col_g[yi + 1];
			gv.f[2] = col_g[yi + 2];
			gv.f[3] = col_g[yi + 3];
			
			bv.f[0] = col_b[yi];
			bv.f[1] = col_b[yi + 1];
			bv.f[2] = col_b[yi + 2];
			bv.f[3] = col_b[yi + 3];
			
			rv = MUL_V4SF(rv, cvy);
			gv = MUL_V4SF(gv, cvy);
			bv = MUL_V4SF(bv, cvy);

			r = SUM_V4SF(rv);
			g = SUM_V4SF(gv);
			b = SUM_V4SF(bv);

			if (r > 255)
				r = 255;
			
			if (r < 0)
				r = 0;
			
			if (g > 255)
				g = 255;
			
			if (g < 0)
				g = 0;
			
			if (b > 255)
				b = 255;
			
			if (b < 0)
				b = 0;
			
			GP_Pixel pix = GP_Pixel_CREATE_RGB888((uint8_t)r, (uint8_t)g, (uint8_t)b);
			GP_PutPixel_Raw_24BPP(res, i, j, pix);
		}
	}
}

void GP_FilterResize_Raw(GP_Context *src, GP_Context *res,
                         GP_ProgressCallback callback,
                         GP_InterpolationType type)
{
	switch (type) {
	case GP_INTER_NN:
		GP_FilterInterpolate_NN(src, res, callback);
	break;
	case GP_INTER_CUBIC:
		GP_FilterInterpolate_Cubic(src, res, callback);
	break;
	}
}

GP_Context *GP_FilterResize(GP_Context *src, GP_ProgressCallback callback,
                            GP_InterpolationType type, GP_Size w, GP_Size h)
{
	GP_Context *res;
	
	if (src->pixel_type != GP_PIXEL_RGB888)
		return NULL;
	
	res = GP_ContextAlloc(w, h, GP_PIXEL_RGB888);
	
	if (res == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	GP_FilterResize_Raw(src, res, callback, type);

	return res;
}
