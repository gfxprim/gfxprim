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

#include <GP_Linear.h>

static float HX[] = {0.17, 0.66, 0.17};
static float HY[] = {0.17, 0.66, 0.17};

GP_Context *GP_FilterBlur(GP_Context *src)
{
	GP_Context *dst;

	if (src->pixel_type != GP_PIXEL_RGB888)
		return NULL;

	dst = GP_ContextCopy(src, 0);

	if (dst == NULL)
		return NULL;

	GP_DEBUG(1, "Linear blur filter (3x3 kernel) %ux%u", src->w, src->h);

	GP_Coord x, y;

	for (y = 0; y < (GP_Coord)dst->h; y++) {
		GP_Pixel pix;
		uint32_t R1, R2, R3;
		uint32_t G1, G2, G3;
		uint32_t B1, B2, B3;
		
		pix = GP_GetPixel_Raw_24BPP(src, 0, y);

		R1 = GP_Pixel_GET_R_RGB888(pix);
		G1 = GP_Pixel_GET_G_RGB888(pix);
		B1 = GP_Pixel_GET_B_RGB888(pix);
		
		R2 = R1;
		G2 = G1;
		B2 = B1;

		for (x = 0; x < (GP_Coord)dst->w; x++) {
			float r, g, b;

			pix = GP_GetPixel_Raw_24BPP(src, x, y);
			
			R3 = GP_Pixel_GET_R_RGB888(pix);
			G3 = GP_Pixel_GET_G_RGB888(pix);
			B3 = GP_Pixel_GET_B_RGB888(pix);

			r = R1 * HX[0] + R2 * HX[1] + R3 * HX[2];
			g = G1 * HX[0] + G2 * HX[1] + G3 * HX[2];
			b = B1 * HX[0] + B2 * HX[1] + B3 * HX[2];

			if (r > 255)
				r = 255;
			if (g > 255)
				g = 255;
			if (b > 255)
				b = 255;

			pix = GP_Pixel_CREATE_RGB888((uint32_t)r, (uint32_t)g, (uint32_t)b);

			GP_PutPixel_Raw_24BPP(dst, x, y, pix);
		
			R1 = R2;
			G1 = G2;
			B1 = B2;
			
			R2 = R3;
			G2 = G3;
			B2 = B3;
		}
	}
	
	for (x = 0; x < (GP_Coord)dst->w; x++) {
		GP_Pixel pix;
		uint32_t R1, R2, R3;
		uint32_t G1, G2, G3;
		uint32_t B1, B2, B3;
		
		pix = GP_GetPixel_Raw_24BPP(src, x, 0);

		R1 = GP_Pixel_GET_R_RGB888(pix);
		G1 = GP_Pixel_GET_G_RGB888(pix);
		B1 = GP_Pixel_GET_B_RGB888(pix);
		
		R2 = R1;
		G2 = G1;
		B2 = B1;

		for (y = 0; y < (GP_Coord)dst->h; y++) {
			float r, g, b;

			pix = GP_GetPixel_Raw_24BPP(src, x, y);
			
			R3 = GP_Pixel_GET_R_RGB888(pix);
			G3 = GP_Pixel_GET_G_RGB888(pix);
			B3 = GP_Pixel_GET_B_RGB888(pix);

			r = R1 * HY[0] + R2 * HY[1] + R3 * HY[2];
			g = G1 * HY[0] + G2 * HY[1] + G3 * HY[2];
			b = B1 * HY[0] + B2 * HY[1] + B3 * HY[2];
			
			if (r > 255)
				r = 255;
			if (g > 255)
				g = 255;
			if (b > 255)
				b = 255;

			pix = GP_Pixel_CREATE_RGB888((uint32_t)r, (uint32_t)g, (uint32_t)b);

			GP_PutPixel_Raw_24BPP(dst, x, y, pix);
		
			R1 = R2;
			G1 = G2;
			B1 = B2;
			
			R2 = R3;
			G2 = G3;
			B2 = B3;
		}
	}
	
	return dst;
}
