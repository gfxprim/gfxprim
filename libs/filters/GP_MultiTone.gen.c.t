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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends "filter.c.t"

%% block descr
Generic Point filer
%% endblock

%% block body

#include <errno.h>

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_TempAlloc.h"
#include "core/GP_MixPixels.h"
#include "core/GP_Debug.h"

#include "filters/GP_MultiTone.h"

%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()
static void init_table_{{ pt.name }}(GP_Pixel table[],
                                     GP_Size table_size,
                                     GP_Pixel pixels[],
                                     GP_Size pixels_size)
{
	unsigned int i;
	unsigned int p = 0;
	float perc;
	float step = 1.00 * table_size / (pixels_size - 1);

	GP_DEBUG(2, "Preparing pixel table %u steps %u pixels, step %.2f",
	         table_size, pixels_size, step);

	for (i = 0; i < table_size; i++) {
		p = 1.00 * i / step;
		perc = i+1;

		while (perc > step)
			perc -= step;

		perc = perc / step;

		table[i] = GP_MIX_PIXELS_{{ pt.name }}(pixels[p+1], pixels[p], 255 * perc);
//		printf("p = %u i = %u PERC %.2f\n", p, i, perc);
//		GP_PixelPrint(table[i], GP_PIXEL_{{ pt.name }});
	}
}
%%  endif
%% endfor

static void init_table(GP_PixelType type,
                       GP_Pixel table[], GP_Size table_size,
                       GP_Pixel pixels[], GP_Size pixels_size)
{
	switch (type) {
%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()
	case GP_PIXEL_{{ pt.name }}:
		init_table_{{ pt.name }}(table, table_size,
		                         pixels, pixels_size);
	break;
%%  endif
%% endfor
	default:
		GP_BUG("Should not be reached");
	break;
	}
}

#include <assert.h>

%% for pt in pixeltypes
%%  if pt.is_gray()
static int multitone_{{ pt.name }}(const GP_Context *const src,
                                   GP_Coord x_src, GP_Coord y_src,
                                   GP_Size w_src, GP_Size h_src,
                                   GP_Context *dst,
                                   GP_Coord x_dst, GP_Coord y_dst,
                                   GP_Pixel pixels[], GP_Size pixels_size,
                                   GP_ProgressCallback *callback)
{
%% set size = pt.chanslist[0].max + 1
	GP_TempAllocCreate(tmp, {{ size }} * sizeof(GP_Pixel));
	GP_Pixel *table = GP_TempAllocGet(tmp, {{ size }} * sizeof(GP_Pixel));

	GP_DEBUG(1, "Duotone filter %ux%u {{ pt.name }} -> %s",
	         w_src, h_src, GP_PixelTypeName(dst->pixel_type));

	init_table(dst->pixel_type, table, {{ size }}, pixels, pixels_size);

	unsigned int x, y;

	for (y = 0; y < h_src; y++) {
		for (x = 0; x < w_src; x++) {
			unsigned int src_x = x_src + x;
			unsigned int src_y = y_src + y;
			unsigned int dst_x = x_dst + x;
			unsigned int dst_y = y_dst + y;

			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, src_x, src_y);

			pix = table[pix];

			GP_PutPixel_Raw(dst, dst_x, dst_y, pix);
		}

		if (GP_ProgressCallbackReport(callback, y, h_src, w_src)) {
			GP_TempAllocFree(tmp);
			errno = ECANCELED;
			return 1;
		}
	}

	GP_TempAllocFree(tmp);
	GP_ProgressCallbackDone(callback);

	return 0;
}

%%  endif
%% endfor

int GP_FilterMultiToneEx(const GP_Context *const src,
                         GP_Coord x_src, GP_Coord y_src,
                         GP_Size w_src, GP_Size h_src,
                         GP_Context *dst,
                         GP_Coord x_dst, GP_Coord y_dst,
                         GP_Pixel pixels[], GP_Size pixels_size,
                         GP_ProgressCallback *callback)
{
	//CHECK DST IS NOT PALETTE PixelHasFlags

	switch (src->pixel_type) {
%% for pt in pixeltypes
%%  if pt.is_gray()
	case GP_PIXEL_{{ pt.name }}:
		return multitone_{{ pt.name }}(src, x_src, y_src,
		                               w_src, h_src, dst,
		                               x_dst, y_dst,
		                               pixels, pixels_size,
		                               callback);
	break;
%%  endif
%% endfor
	default:
		errno = EINVAL;
		return -1;
	}
}

GP_Context *GP_FilterMultiToneExAlloc(const GP_Context *const src,
                                      GP_Coord x_src, GP_Coord y_src,
                                      GP_Size w_src, GP_Size h_src,
                                      GP_PixelType dst_pixel_type,
                                      GP_Pixel pixels[], GP_Size pixels_size,
                                      GP_ProgressCallback *callback)
{
	GP_Context *res;
	int err;

	res = GP_ContextAlloc(w_src, h_src, dst_pixel_type);

	if (!res) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	if (GP_FilterMultiToneEx(src, x_src, y_src, w_src, h_src, res, 0, 0,
	                         pixels, pixels_size, callback)) {
		err = errno;
		GP_ContextFree(res);
		errno = err;
		return NULL;
	}

	return res;
}

%% endblock body
