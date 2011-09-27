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

GP_Context *GP_Scale(GP_Context *src, GP_Size w, GP_Size h)
{
	GP_Context *dst;
	float col_r[src->h], col_g[src->h], col_b[src->h];
	uint32_t i, j, k;
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
			uint8_t r[4], g[4], b[4];
			float c[4];
				
			for (k = 0; k < 4; k++) {
				GP_Pixel pix;
				idx = x + k - 1;
				
				if (idx < 0)
					idx = 0;

				if (idx > (int)src->w - 1)
					idx = src->w - 1;

				pix = GP_GetPixel_Raw_24BPP(src, idx, j);
				
				r[k] = GP_Pixel_GET_R_RGB888(pix);
				g[k] = GP_Pixel_GET_G_RGB888(pix);
				b[k] = GP_Pixel_GET_B_RGB888(pix);
			
				c[k] = cubic(x - idx);
			}

			col_r[j] = r[0] * c[0] + r[1] * c[1] + r[2] * c[2] + r[3] * c[3];
			col_g[j] = g[0] * c[0] + g[1] * c[1] + g[2] * c[2] + g[3] * c[3];
			col_b[j] = b[0] * c[0] + b[1] * c[1] + b[2] * c[2] + b[3] * c[3];

		}

		/* now interpolate column for new image */
		for (j = 0; j < h; j++) {
			float y = (1.00 * j / h) * src->h + 0.5;
			float r = 0, g = 0, b = 0, c;
		
			for (k = 0; k < 4; k++) {
				idx = y + k - 1;
				
				if (idx < 0)
					idx = 0;

				if (idx > (int)src->h - 1)
					idx = src->h - 1;

				c = cubic(y - idx);
		
				r += col_r[idx] * c;
				g += col_g[idx] * c;
				b += col_b[idx] * c;
			}
			
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
