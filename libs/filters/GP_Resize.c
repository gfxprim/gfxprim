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

int GP_FilterInterpolate_NN(const GP_Context *src, GP_Context *dst,
                            GP_ProgressCallback *callback)
{
	GP_Coord x, y;
	
	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	for (y = 0; y < (GP_Coord)dst->h; y++) {
		for (x = 0; x < (GP_Coord)dst->w; x++) {
			GP_Coord xi = (1.00 * x / dst->w) * src->w;
			GP_Coord yi = (1.00 * y / dst->h) * src->h;
			
			GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, xi, yi);

			GP_PutPixel_Raw_24BPP(dst, x, y, pix);
		}
	
		if (GP_ProgressCallbackReport(callback, y, dst->h, dst->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
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

#define GP_USE_GCC_VECTOR

#ifdef GP_USE_GCC_VECTOR
#define MUL_V4SF(a, b) ({v4f ret; ret.v = (a).v * (b).v; ret;})
#else
#define MUL_V4SF(a, b) ({v4f ret; \
                         ret.f[0] = (a).f[0] * (b).f[0]; \
                         ret.f[1] = (a).f[1] * (b).f[1]; \
                         ret.f[2] = (a).f[2] * (b).f[2]; \
                         ret.f[3] = (a).f[3] * (b).f[3]; \
						 ret;})
#endif /* GP_USE_GCC_VECTOR */

#define SUM_V4SF(a)    ((a).f[0] + (a).f[1] + (a).f[2] + (a).f[3])

int GP_FilterInterpolate_Cubic(const GP_Context *src, GP_Context *dst,
                               GP_ProgressCallback *callback)
{
	float col_r[src->h], col_g[src->h], col_b[src->h];
	uint32_t i, j;

	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	for (i = 0; i < dst->w; i++) {
		float x = (1.00 * i / dst->w) * src->w;
		v4f cvx;
		int xi = x - 1;
		
		if (xi < 0)
			xi = 0;

		if (xi > (int)src->w - 4)
			xi = src->w - 4;
		
		cvx.f[0] = cubic(xi - x);
		cvx.f[1] = cubic(xi + 1 - x);
		cvx.f[2] = cubic(xi + 2 - x);
		cvx.f[3] = cubic(xi + 3 - x);
		
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
		for (j = 0; j < dst->h; j++) {
			float y = (1.00 * j / dst->h) * src->h;
			v4f cvy, rv, gv, bv;
			float r, g, b;
			int yi = y - 1;
			
			if (yi < 0)
				yi = 0;

			if (yi > (int)src->h - 4)
				yi = src->h - 4;
			
			cvy.f[0] = cubic(yi - y);
			cvy.f[1] = cubic(yi + 1 - y);
			cvy.f[2] = cubic(yi + 2 - y);
			cvy.f[3] = cubic(yi + 3 - y);
				
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
			GP_PutPixel_Raw_24BPP(dst, i, j, pix);
		}
		
		if (GP_ProgressCallbackReport(callback, i, dst->w, dst->h))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

#define MUL 2048

#define MUL_I(a, b) ({ \
	a[0] *= b[0]; \
	a[1] *= b[1]; \
	a[2] *= b[2]; \
	a[3] *= b[3]; \
})

#define SUM_I(a) \
	((a)[0] + (a)[1] + (a)[2] + (a)[3])

int GP_FilterInterpolate_CubicInt(const GP_Context *src, GP_Context *dst,
                                  GP_ProgressCallback *callback)
{
	int32_t col_r[src->h], col_g[src->h], col_b[src->h];
	uint32_t i, j;

	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	for (i = 0; i < dst->w; i++) {
		float x = (1.00 * i / dst->w) * src->w;
		int32_t cvx[4];
		int xi = x - 1;
		
		if (xi < 0)
			xi = 0;

		if (xi > (int)src->w - 4)
			xi = src->w - 4;
		
		cvx[0] = cubic(xi - x) * MUL + 0.5;
		cvx[1] = cubic(xi + 1 - x) * MUL + 0.5;
		cvx[2] = cubic(xi + 2 - x) * MUL + 0.5;
		cvx[3] = cubic(xi + 3 - x) * MUL + 0.5;
		
		/* Generate interpolated column */
		for (j = 0; j < src->h; j++) {
			int32_t rv[4], gv[4], bv[4];
			GP_Pixel pix[4];

			pix[0] = GP_GetPixel_Raw_24BPP(src, xi, j);
			pix[1] = GP_GetPixel_Raw_24BPP(src, xi + 1, j);
			pix[2] = GP_GetPixel_Raw_24BPP(src, xi + 2, j);
			pix[3] = GP_GetPixel_Raw_24BPP(src, xi + 3, j);
				
			rv[0] = GP_Pixel_GET_R_RGB888(pix[0]);
			rv[1] = GP_Pixel_GET_R_RGB888(pix[1]);
			rv[2] = GP_Pixel_GET_R_RGB888(pix[2]);
			rv[3] = GP_Pixel_GET_R_RGB888(pix[3]);
			
			gv[0] = GP_Pixel_GET_G_RGB888(pix[0]);
			gv[1] = GP_Pixel_GET_G_RGB888(pix[1]);
			gv[2] = GP_Pixel_GET_G_RGB888(pix[2]);
			gv[3] = GP_Pixel_GET_G_RGB888(pix[3]);
			
			bv[0] = GP_Pixel_GET_B_RGB888(pix[0]);
			bv[1] = GP_Pixel_GET_B_RGB888(pix[1]);
			bv[2] = GP_Pixel_GET_B_RGB888(pix[2]);
			bv[3] = GP_Pixel_GET_B_RGB888(pix[3]);
			
			MUL_I(rv, cvx);
			MUL_I(gv, cvx);
			MUL_I(bv, cvx);

			col_r[j] = SUM_I(rv);
			col_g[j] = SUM_I(gv);
			col_b[j] = SUM_I(bv);
		}

		/* now interpolate column for new image */
		for (j = 0; j < dst->h; j++) {
			float y = (1.00 * j / dst->h) * src->h;
			int32_t cvy[4], rv[4], gv[4], bv[4];
			int32_t r, g, b;
			int yi = y - 1;
			
			if (yi < 0)
				yi = 0;

			if (yi > (int)src->h - 4)
				yi = src->h - 4;
			
			cvy[0] = cubic(yi - y) * MUL + 0.5;
			cvy[1] = cubic(yi + 1 - y) * MUL + 0.5;
			cvy[2] = cubic(yi + 2 - y) * MUL + 0.5;
			cvy[3] = cubic(yi + 3 - y) * MUL + 0.5;
			
			rv[0] = col_r[yi];
			rv[1] = col_r[yi + 1];
			rv[2] = col_r[yi + 2];
			rv[3] = col_r[yi + 3];
			
			gv[0] = col_g[yi];
			gv[1] = col_g[yi + 1];
			gv[2] = col_g[yi + 2];
			gv[3] = col_g[yi + 3];
			
			bv[0] = col_b[yi];
			bv[1] = col_b[yi + 1];
			bv[2] = col_b[yi + 2];
			bv[3] = col_b[yi + 3];
			
			MUL_I(rv, cvy);
			MUL_I(gv, cvy);
			MUL_I(bv, cvy);

			r = (SUM_I(rv) + MUL*MUL/2) / MUL / MUL;
			g = (SUM_I(gv) + MUL*MUL/2) / MUL / MUL;
			b = (SUM_I(bv) + MUL*MUL/2) / MUL / MUL;

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
			GP_PutPixel_Raw_24BPP(dst, i, j, pix);
		}
		
		if (GP_ProgressCallbackReport(callback, i, dst->w, dst->h))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

int GP_FilterInterpolate_CubicInt2(const GP_Context *src, GP_Context *dst,
                                   GP_ProgressCallback *callback)
{
	int32_t col_r[src->w], col_g[src->w], col_b[src->w];
	uint32_t i, j;

	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	for (i = 0; i < dst->h; i++) {
		float y = (1.00 * i / dst->h) * src->h;
		int32_t cvy[4];
		int yi = y - 1;
		
		if (yi < 0)
			yi = 0;

		if (yi > (int)src->h - 4)
			yi = src->h - 4;
		
		cvy[0] = cubic(yi - y) * MUL + 0.5;
		cvy[1] = cubic(yi + 1 - y) * MUL + 0.5;
		cvy[2] = cubic(yi + 2 - y) * MUL + 0.5;
		cvy[3] = cubic(yi + 3 - y) * MUL + 0.5;
		
		/* Generate interpolated row */
		for (j = 0; j < src->w; j++) {
			int32_t rv[4], gv[4], bv[4];
			GP_Pixel pix[4];

			pix[0] = GP_GetPixel_Raw_24BPP(src, j, yi);
			pix[1] = GP_GetPixel_Raw_24BPP(src, j, yi + 1);
			pix[2] = GP_GetPixel_Raw_24BPP(src, j, yi + 2);
			pix[3] = GP_GetPixel_Raw_24BPP(src, j, yi + 3);
				
			rv[0] = GP_Pixel_GET_R_RGB888(pix[0]);
			rv[1] = GP_Pixel_GET_R_RGB888(pix[1]);
			rv[2] = GP_Pixel_GET_R_RGB888(pix[2]);
			rv[3] = GP_Pixel_GET_R_RGB888(pix[3]);
			
			gv[0] = GP_Pixel_GET_G_RGB888(pix[0]);
			gv[1] = GP_Pixel_GET_G_RGB888(pix[1]);
			gv[2] = GP_Pixel_GET_G_RGB888(pix[2]);
			gv[3] = GP_Pixel_GET_G_RGB888(pix[3]);
			
			bv[0] = GP_Pixel_GET_B_RGB888(pix[0]);
			bv[1] = GP_Pixel_GET_B_RGB888(pix[1]);
			bv[2] = GP_Pixel_GET_B_RGB888(pix[2]);
			bv[3] = GP_Pixel_GET_B_RGB888(pix[3]);
			
			MUL_I(rv, cvy);
			MUL_I(gv, cvy);
			MUL_I(bv, cvy);

			col_r[j] = SUM_I(rv);
			col_g[j] = SUM_I(gv);
			col_b[j] = SUM_I(bv);
		}

		/* now interpolate column for new image */
		for (j = 0; j < dst->w; j++) {
			float x = (1.00 * j / dst->w) * src->w;
			int32_t cvx[4], rv[4], gv[4], bv[4];
			int32_t r, g, b;
			int xi = x - 1;
			
			if (xi < 0)
				xi = 0;

			if (xi > (int)src->w - 4)
				xi = src->w - 4;
			
			cvx[0] = cubic(xi - x) * MUL + 0.5;
			cvx[1] = cubic(xi + 1 - x) * MUL + 0.5;
			cvx[2] = cubic(xi + 2 - x) * MUL + 0.5;
			cvx[3] = cubic(xi + 3 - x) * MUL + 0.5;
			
			rv[0] = col_r[xi];
			rv[1] = col_r[xi + 1];
			rv[2] = col_r[xi + 2];
			rv[3] = col_r[xi + 3];
			
			gv[0] = col_g[xi];
			gv[1] = col_g[xi + 1];
			gv[2] = col_g[xi + 2];
			gv[3] = col_g[xi + 3];
			
			bv[0] = col_b[xi];
			bv[1] = col_b[xi + 1];
			bv[2] = col_b[xi + 2];
			bv[3] = col_b[xi + 3];
			
			MUL_I(rv, cvx);
			MUL_I(gv, cvx);
			MUL_I(bv, cvx);

			r = (SUM_I(rv) + MUL*MUL/2) / MUL / MUL;
			g = (SUM_I(gv) + MUL*MUL/2) / MUL / MUL;
			b = (SUM_I(bv) + MUL*MUL/2) / MUL / MUL;

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
			GP_PutPixel_Raw_24BPP(dst, j, i, pix);
		}
		
		if (GP_ProgressCallbackReport(callback, i, dst->h, dst->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

int GP_FilterResize_Raw(const GP_Context *src, GP_Context *dst,
                        GP_InterpolationType type,
                        GP_ProgressCallback *callback)
{
	switch (type) {
	case GP_INTERP_NN:
		return GP_FilterInterpolate_NN(src, dst, callback);
	case GP_INTERP_CUBIC:
		return GP_FilterInterpolate_Cubic(src, dst, callback);
	case GP_INTERP_CUBIC_INT:
		return GP_FilterInterpolate_CubicInt2(src, dst, callback);
	}

	return 1;
}

GP_Context *GP_FilterResize(const GP_Context *src, GP_Context *dst,
                            GP_InterpolationType type,
                            GP_Size w, GP_Size h,
                            GP_ProgressCallback *callback)
{
	GP_Context sub, *res;

	/* TODO: Templatize */
	if (src->pixel_type != GP_PIXEL_RGB888)
		return NULL;

	if (dst == NULL) {
		res = GP_ContextAlloc(w, h, src->pixel_type);

		if (res == NULL)
			return NULL;
	} else {
		GP_ASSERT(src->pixel_type == dst->pixel_type,
		          "The src and dst pixel types must match");
		/*
		 * The size of w and h is asserted in subcontext initalization
		 */	
		res = GP_ContextSubContext(dst, &sub, 0, 0, w, h);
	}

	/*
	 * Operation was aborted by progress callback.
	 * 
	 * Free any alloacted data and exit.
	 */
	if (GP_FilterResize_Raw(src, res, type, callback)) {
		GP_DEBUG(1, "Operation aborted");
		
		if (dst == NULL)
			GP_ContextFree(dst);

		return NULL;
	}

	return dst == NULL ? res : dst;
}
