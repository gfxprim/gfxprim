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

#include "histogram.h"

void histogram_to_png(const GP_Context *src, const char *filename)
{
	GP_FILTER_PARAMS(src->pixel_type, params);
	
	GP_FilterHistogramAlloc(src->pixel_type, params);
	GP_FilterHistogram(src, params, NULL);

	unsigned int i, j;

	GP_Context *res = GP_ContextAlloc(257*4, 256, GP_PIXEL_RGB888);
	
	GP_Fill(res, 0xffffff);

	GP_Histogram *hist_r;
	hist_r = (GP_FilterParamChannel(params, "R"))->val.ptr;
	
	for (i = 0; i < hist_r->len; i++)
		GP_VLineXYH(res, i, 256, -255.00 * hist_r->hist[i] / hist_r->max + 0.5 , 0xff0000);
	
	GP_Histogram *hist_g;
	hist_g = (GP_FilterParamChannel(params, "G"))->val.ptr;
	
	for (i = 0; i < hist_g->len; i++)
		GP_VLineXYH(res, i+257, 256, -255.00 * hist_g->hist[i] / hist_g->max + 0.5 , 0x00ff00);
	
	GP_Histogram *hist_b;
	hist_b = (GP_FilterParamChannel(params, "B"))->val.ptr;
	
	for (i = 0; i < hist_b->len; i++)
		GP_VLineXYH(res, i+514, 256, -255.00 * hist_b->hist[i] / hist_b->max + 0.5 , 0x0000ff);

	uint32_t max = GP_MAX(hist_r->max, hist_g->max);

	max = GP_MAX(max, hist_b->max);

	for (i = 0; i < hist_r->len; i++) {
		for (j = 0; j < hist_r->len; j++) {
			GP_Pixel pix = 0;
			
			if (255 * hist_r->hist[i] / max + 0.5 > j)
				pix |= 0xff0000;
			
			if (255 * hist_g->hist[i] / max + 0.5 > j)
				pix |= 0x00ff00;

			if (255 * hist_b->hist[i] / max + 0.5 > j)
				pix |= 0x0000ff;

			GP_PutPixel(res, i+771, 256-j, pix);
		}
	}

	GP_SavePNG(res, filename, NULL);

	GP_ContextFree(res);
	GP_FilterHistogramFree(params);
}
