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

#define A 0.5

float cubic(float x)
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

GP_Context *GP_Scale(GP_Context *src, GP_Size w, GP_Size h)
{
	GP_Context *dst;
	float col_r[src->h], col_g[src->h], col_b[src->h];
	uint32_t i, j;
	int idx;

	if (src->pixel_type != GP_PIXEL_RGB888)
		return NULL;
	
	dst = GP_ContextAlloc(w, h, GP_PIXEL_RGB888);
	
	if (dst == NULL)
		return NULL;

	for (i = 0; i < w; i++) {
		float x = (1.00 * i / w) * src->w + 0.5;
	
		/* Generate interpolated column */
		for (j = 0; j < src->h; j++) {
			v4f cv, rv, gv, bv;
			GP_Pixel pix[4];

			idx = x - 1;
				
			if (idx < 0)
				idx = 0;

			if (idx > (int)src->w - 4)
				idx = src->w - 4;

			pix[0] = GP_GetPixel_Raw_24BPP(src, idx, j);
			pix[1] = GP_GetPixel_Raw_24BPP(src, idx + 1, j);
			pix[2] = GP_GetPixel_Raw_24BPP(src, idx + 2, j);
			pix[3] = GP_GetPixel_Raw_24BPP(src, idx + 3, j);
				
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

			cv.f[0] = cubic(x - idx);
			cv.f[1] = cubic(x - idx - 1);
			cv.f[2] = cubic(x - idx - 2);
			cv.f[3] = cubic(x - idx - 3);
			
			rv = MUL_V4SF(rv, cv);
			gv = MUL_V4SF(gv, cv);
			bv = MUL_V4SF(bv, cv);
			
			col_r[j] = SUM_V4SF(rv);
			col_g[j] = SUM_V4SF(gv);
			col_b[j] = SUM_V4SF(bv);
		}

		/* now interpolate column for new image */
		for (j = 0; j < h; j++) {
			float y = (1.00 * j / h) * src->h + 0.5;
			v4f cv, rv, gv, bv;
			float r, g, b;

			idx = y - 1;
				
			if (idx < 0)
				idx = 0;

			if (idx > (int)src->h - 4)
				idx = src->h - 4;
			
			rv.f[0] = col_r[idx];
			rv.f[1] = col_r[idx + 1];
			rv.f[2] = col_r[idx + 2];
			rv.f[3] = col_r[idx + 3];
			
			gv.f[0] = col_g[idx];
			gv.f[1] = col_g[idx + 1];
			gv.f[2] = col_g[idx + 2];
			gv.f[3] = col_g[idx + 3];
			
			bv.f[0] = col_b[idx];
			bv.f[1] = col_b[idx + 1];
			bv.f[2] = col_b[idx + 2];
			bv.f[3] = col_b[idx + 3];
			
			cv.f[0] = cubic(y - idx);
			cv.f[1] = cubic(y - idx - 1);
			cv.f[2] = cubic(y - idx - 2);
			cv.f[3] = cubic(y - idx - 3);

			rv = MUL_V4SF(rv, cv);
			gv = MUL_V4SF(gv, cv);
			bv = MUL_V4SF(bv, cv);

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
	}

	return dst;
}
