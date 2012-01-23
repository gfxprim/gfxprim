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

#include <GP_Context.h>
#include <GP_GetPutPixel.h>

#include <GP_Debug.h>

#include <GP_Resize.h>

int GP_FilterInterpolate_NN(const GP_Context *src, GP_Context *dst,
                            GP_ProgressCallback *callback)
{
	uint32_t xmap[dst->w];
	uint32_t ymap[dst->h];
	uint32_t i;
	GP_Coord x, y;
	
	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	/* Pre-compute mapping for interpolation */
	uint32_t xstep = (src->w << 16) / dst->w;

	for (i = 0; i < dst->w + 1; i++)
		xmap[i] = ((i * xstep) + (1<<15)) >> 16;
	
	uint32_t ystep = (src->h << 16) / dst->h;

	for (i = 0; i < dst->h + 1; i++)
		ymap[i] = ((i * ystep) + (1<<15)) >> 16;

	/* Interpolate */
	for (y = 0; y < (GP_Coord)dst->h; y++) {
		for (x = 0; x < (GP_Coord)dst->w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, xmap[x], ymap[y]);

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

#define CLAMP(val) do {    \
	if (val < 0)       \
		val = 0;   \
	if (val > 255)     \
		val = 255; \
} while (0)

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
		int xi[4];
		
		xi[0] = floor(x - 1);
		xi[1] = x;
		xi[2] = x + 1;
		xi[3] = x + 2;
		
		cvx.f[0] = cubic(xi[0] - x);
		cvx.f[1] = cubic(xi[1] - x);
		cvx.f[2] = cubic(xi[2] - x);
		cvx.f[3] = cubic(xi[3] - x);
		
		if (xi[0] < 0)
			xi[0] = 0;

		if (xi[2] >= (int)src->w)
			xi[2] = src->w - 1;
		
		if (xi[3] >= (int)src->w)
			xi[3] = src->w - 1;
		
		/* Generate interpolated column */
		for (j = 0; j < src->h; j++) {
			v4f rv, gv, bv;
			GP_Pixel pix[4];

			pix[0] = GP_GetPixel_Raw_24BPP(src, xi[0], j);
			pix[1] = GP_GetPixel_Raw_24BPP(src, xi[1], j);
			pix[2] = GP_GetPixel_Raw_24BPP(src, xi[2], j);
			pix[3] = GP_GetPixel_Raw_24BPP(src, xi[3], j);
				
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
			int yi[4];
		
			yi[0] = floor(y - 1);
			yi[1] = y;
			yi[2] = y + 1;
			yi[3] = y + 2;
			
			cvy.f[0] = cubic(yi[0] - y);
			cvy.f[1] = cubic(yi[1] - y);
			cvy.f[2] = cubic(yi[2] - y);
			cvy.f[3] = cubic(yi[3] - y);
		
			if (yi[0] < 0)
				yi[0] = 0;
		
			if (yi[2] >= (int)src->h)
				yi[2] = src->h - 1;
		
			if (yi[3] >= (int)src->h)
				yi[3] = src->h - 1;
			
			rv.f[0] = col_r[yi[0]];
			rv.f[1] = col_r[yi[1]];
			rv.f[2] = col_r[yi[2]];
			rv.f[3] = col_r[yi[3]];
			
			gv.f[0] = col_g[yi[0]];
			gv.f[1] = col_g[yi[1]];
			gv.f[2] = col_g[yi[2]];
			gv.f[3] = col_g[yi[3]];
			
			bv.f[0] = col_b[yi[0]];
			bv.f[1] = col_b[yi[1]];
			bv.f[2] = col_b[yi[2]];
			bv.f[3] = col_b[yi[3]];
			
			rv = MUL_V4SF(rv, cvy);
			gv = MUL_V4SF(gv, cvy);
			bv = MUL_V4SF(bv, cvy);

			r = SUM_V4SF(rv);
			g = SUM_V4SF(gv);
			b = SUM_V4SF(bv);

			CLAMP(r);
			CLAMP(g);
			CLAMP(b);

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
	int32_t col_r[src->w], col_g[src->w], col_b[src->w];
	uint32_t i, j;

	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	for (i = 0; i < dst->h; i++) {
		float y = (1.00 * i / dst->h) * src->h;
		int32_t cvy[4];
		int yi[4];
		
		yi[0] = floor(y - 1);
		yi[1] = y;
		yi[2] = y + 1;
		yi[3] = y + 2;
		
		cvy[0] = cubic(yi[0] - y) * MUL + 0.5;
		cvy[1] = cubic(yi[1] - y) * MUL + 0.5;
		cvy[2] = cubic(yi[2] - y) * MUL + 0.5;
		cvy[3] = cubic(yi[3] - y) * MUL + 0.5;
		
		if (yi[0] < 0)
			yi[0] = 0;
		
		if (yi[2] >= (int)src->h)
			yi[2] = src->h - 1;
		
		if (yi[3] >= (int)src->h)
			yi[3] = src->h - 1;

		/* Generate interpolated row */
		for (j = 0; j < src->w; j++) {
			int32_t rv[4], gv[4], bv[4];
			GP_Pixel pix[4];

			pix[0] = GP_GetPixel_Raw_24BPP(src, j, yi[0]);
			pix[1] = GP_GetPixel_Raw_24BPP(src, j, yi[1]);
			pix[2] = GP_GetPixel_Raw_24BPP(src, j, yi[2]);
			pix[3] = GP_GetPixel_Raw_24BPP(src, j, yi[3]);
				
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
			int xi[4];
		
			xi[0] = floor(x - 1);
			xi[1] = x;
			xi[2] = x + 1;
			xi[3] = x + 2;
			
			cvx[0] = cubic(xi[0] - x) * MUL + 0.5;
			cvx[1] = cubic(xi[1] - x) * MUL + 0.5;
			cvx[2] = cubic(xi[2] - x) * MUL + 0.5;
			cvx[3] = cubic(xi[3] - x) * MUL + 0.5;
			
			if (xi[0] < 0)
				xi[0] = 0;

			if (xi[2] >= (int)src->w)
				xi[2] = src->w - 1;
			
			if (xi[3] >= (int)src->w)
				xi[3] = src->w - 1;

			rv[0] = col_r[xi[0]];
			rv[1] = col_r[xi[1]];
			rv[2] = col_r[xi[2]];
			rv[3] = col_r[xi[3]];
			
			gv[0] = col_g[xi[0]];
			gv[1] = col_g[xi[1]];
			gv[2] = col_g[xi[2]];
			gv[3] = col_g[xi[3]];
			
			bv[0] = col_b[xi[0]];
			bv[1] = col_b[xi[1]];
			bv[2] = col_b[xi[2]];
			bv[3] = col_b[xi[3]];
			
			MUL_I(rv, cvx);
			MUL_I(gv, cvx);
			MUL_I(bv, cvx);

			r = (SUM_I(rv) + MUL*MUL/2) / MUL / MUL;
			g = (SUM_I(gv) + MUL*MUL/2) / MUL / MUL;
			b = (SUM_I(bv) + MUL*MUL/2) / MUL / MUL;

			CLAMP(r);
			CLAMP(g);
			CLAMP(b);
			
			GP_Pixel pix = GP_Pixel_CREATE_RGB888((uint8_t)r, (uint8_t)g, (uint8_t)b);
			GP_PutPixel_Raw_24BPP(dst, j, i, pix);
		}
		
		if (GP_ProgressCallbackReport(callback, i, dst->h, dst->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

int GP_FilterInterpolate_LinearInt(const GP_Context *src, GP_Context *dst,
                                   GP_ProgressCallback *callback)
{
	uint32_t xmap[dst->w + 1];
	uint32_t ymap[dst->h + 1];
	uint8_t  xoff[dst->w + 1];
	uint8_t  yoff[dst->h + 1];
	uint32_t i, j;
	
	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	/* Pre-compute mapping for interpolation */
	uint32_t xstep = (src->w << 16) / dst->w;

	for (i = 0; i < dst->w + 1; i++) {
		uint32_t val = i * xstep;
		xmap[i] = val >> 16;
		xoff[i] = (val >> 8) & 0xff;
	}

	uint32_t ystep = (src->h << 16) / dst->h;

	for (i = 0; i < dst->h + 1; i++) {
		uint32_t val = i * ystep;
		ymap[i] = val >> 16;
		yoff[i] = (val >> 8) & 0xff; 
	}

	/* Interpolate */
	for (i = 0; i < dst->h; i++) {
		for (j = 0; j < dst->w; j++) {
			GP_Pixel pix00, pix01, pix10, pix11;
			uint32_t r0, r1, g0, g1, b0, b1;
			GP_Coord x0, x1, y0, y1;
			
			x0 = xmap[j];
			x1 = xmap[j] + 1;

			if (x1 >= (GP_Coord)src->w)
				x1 = src->w - 1;
		
			y0 = ymap[i];
			y1 = ymap[i] + 1;

			if (y1 >= (GP_Coord)src->h)
				y1 = src->h - 1;

			pix00 = GP_GetPixel_Raw_24BPP(src, x0, y0);
			pix10 = GP_GetPixel_Raw_24BPP(src, x1, y0);
			pix01 = GP_GetPixel_Raw_24BPP(src, x0, y1);
			pix11 = GP_GetPixel_Raw_24BPP(src, x1, y1);

			r0 = GP_Pixel_GET_R_RGB888(pix00) * (255 - xoff[j]);
			g0 = GP_Pixel_GET_G_RGB888(pix00) * (255 - xoff[j]);
			b0 = GP_Pixel_GET_B_RGB888(pix00) * (255 - xoff[j]);
		
			r0 += GP_Pixel_GET_R_RGB888(pix10) * xoff[j];
			g0 += GP_Pixel_GET_G_RGB888(pix10) * xoff[j];
			b0 += GP_Pixel_GET_B_RGB888(pix10) * xoff[j];
			
			r1 = GP_Pixel_GET_R_RGB888(pix01) * (255 - xoff[j]);
			g1 = GP_Pixel_GET_G_RGB888(pix01) * (255 - xoff[j]);
			b1 = GP_Pixel_GET_B_RGB888(pix01) * (255 - xoff[j]);
		
			r1 += GP_Pixel_GET_R_RGB888(pix11) * xoff[j];
			g1 += GP_Pixel_GET_G_RGB888(pix11) * xoff[j];
			b1 += GP_Pixel_GET_B_RGB888(pix11) * xoff[j];
		
			r0 = (r1 * yoff[i] + r0 * (255 - yoff[i])) >> 16;
			g0 = (g1 * yoff[i] + g0 * (255 - yoff[i])) >> 16;
			b0 = (b1 * yoff[i] + b0 * (255 - yoff[i])) >> 16;

			GP_PutPixel_Raw_24BPP(dst, j, i,
			                      GP_Pixel_CREATE_RGB888(r0, g0, b0));
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
	case GP_INTERP_LINEAR_INT:
		return GP_FilterInterpolate_LinearInt(src, dst, callback);
	case GP_INTERP_CUBIC:
		return GP_FilterInterpolate_Cubic(src, dst, callback);
	case GP_INTERP_CUBIC_INT:
		return GP_FilterInterpolate_CubicInt(src, dst, callback);
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
