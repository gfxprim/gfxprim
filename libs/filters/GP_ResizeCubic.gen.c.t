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

{% block descr %}Cubic resampling{% endblock %}

%% block body

#include <math.h>

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_Gamma.h"
#include "core/GP_Clamp.h"
#include "core/GP_Debug.h"

#include "GP_Cubic.h"

#include "GP_Resize.h"

#define MUL 1024

#define MUL_I(a, b) ({ \
	a[0] *= b[0]; \
	a[1] *= b[1]; \
	a[2] *= b[2]; \
	a[3] *= b[3]; \
})

#define SUM_I(a) \
	((a)[0] + (a)[1] + (a)[2] + (a)[3])

%% for pt in pixeltypes
%% if not pt.is_unknown() and not pt.is_palette()

static int GP_FilterResizeCubicInt_{{ pt.name }}_Raw(const GP_Context *src,
	GP_Context *dst, GP_ProgressCallback *callback)
{
	%% for c in pt.chanslist
	int32_t col_{{ c[0] }}[src->w];
	%% endfor

	uint32_t i, j;

	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

{{ fetch_gamma_tables(pt, "src") }}

	/* pre-generate x mapping and constants */
	int32_t xmap[dst->w][4];
	int32_t xmap_c[dst->w][4];

	for (i = 0; i < dst->w; i++) {
		float x = (1.00 * i / (dst->w - 1)) * (src->w - 1);

		xmap[i][0] = floor(x - 1);
		xmap[i][1] = x;
		xmap[i][2] = x + 1;
		xmap[i][3] = x + 2;

		xmap_c[i][0] = cubic_int((xmap[i][0] - x) * MUL + 0.5);
		xmap_c[i][1] = cubic_int((xmap[i][1] - x) * MUL + 0.5);
		xmap_c[i][2] = cubic_int((xmap[i][2] - x) * MUL + 0.5);
		xmap_c[i][3] = cubic_int((xmap[i][3] - x) * MUL + 0.5);

		xmap[i][0] = GP_MAX(xmap[i][0], 0);
		xmap[i][2] = GP_MIN(xmap[i][2], (int)src->w - 1);
		xmap[i][3] = GP_MIN(xmap[i][3], (int)src->w - 1);
	}

	/* cubic resampling */
	for (i = 0; i < dst->h; i++) {
		float y = (1.00 * i / (dst->h - 1)) * (src->h - 1);
		int32_t cvy[4];
		int yi[4];

		yi[0] = floor(y - 1);
		yi[1] = y;
		yi[2] = y + 1;
		yi[3] = y + 2;

		cvy[0] = cubic_int((yi[0] - y) * MUL + 0.5);
		cvy[1] = cubic_int((yi[1] - y) * MUL + 0.5);
		cvy[2] = cubic_int((yi[2] - y) * MUL + 0.5);
		cvy[3] = cubic_int((yi[3] - y) * MUL + 0.5);

		yi[0] = GP_MAX(yi[0], 0);
		yi[2] = GP_MIN(yi[2], (int)src->h - 1);
		yi[3] = GP_MIN(yi[3], (int)src->h - 1);	

		/* Generate interpolated row */
		for (j = 0; j < src->w; j++) {
			%% for c in pt.chanslist
			int32_t {{ c[0] }}v[4];
			%% endfor
			GP_Pixel pix[4];

			pix[0] = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, j, yi[0]);
			pix[1] = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, j, yi[1]);
			pix[2] = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, j, yi[2]);
			pix[3] = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, j, yi[3]);

			%% for c in pt.chanslist
			{{ c[0] }}v[0] = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix[0]);
			{{ c[0] }}v[1] = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix[1]);
			{{ c[0] }}v[2] = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix[2]);
			{{ c[0] }}v[3] = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix[3]);
			%% endfor

			if (src->gamma) {
				%% for c in pt.chanslist
				{{ c[0] }}v[0] = {{ c[0] }}_2_LIN[{{ c[0] }}v[0]];
				{{ c[0] }}v[1] = {{ c[0] }}_2_LIN[{{ c[0] }}v[1]];
				{{ c[0] }}v[2] = {{ c[0] }}_2_LIN[{{ c[0] }}v[2]];
				{{ c[0] }}v[3] = {{ c[0] }}_2_LIN[{{ c[0] }}v[3]];
				%% endfor
			}

			%% for c in pt.chanslist
			MUL_I({{ c[0] }}v, cvy);
			%% endfor

			%% for c in pt.chanslist
			col_{{ c[0] }}[j] = SUM_I({{ c[0] }}v);
			%% endfor
		}

		/* now interpolate column for new image */
		for (j = 0; j < dst->w; j++) {
			%% for c in pt.chanslist
			int32_t {{ c[0] }}v[4];
			int32_t {{ c[0] }};
			%% endfor

			%% for c in pt.chanslist
			{{ c[0] }}v[0] = col_{{ c[0] }}[xmap[j][0]];
			{{ c[0] }}v[1] = col_{{ c[0] }}[xmap[j][1]];
			{{ c[0] }}v[2] = col_{{ c[0] }}[xmap[j][2]];
			{{ c[0] }}v[3] = col_{{ c[0] }}[xmap[j][3]];
			%% endfor

			%% for c in pt.chanslist
			MUL_I({{ c[0] }}v, xmap_c[j]);
			%% endfor

			%% for c in pt.chanslist
			{{ c[0] }} = (SUM_I({{ c[0] }}v) + MUL*MUL/2) / MUL / MUL;
			%% endfor

			if (src->gamma) {
			%% for c in pt.chanslist
				{{ c[0] }} = GP_CLAMP_GENERIC({{ c[0] }}, 0, {{ 2 ** (c[2] + 2) - 1 }});
			%% endfor
			%% for c in pt.chanslist
				{{ c[0] }} = {{ c[0] }}_2_GAMMA[{{ c[0] }}];
			%% endfor
			} else {
			%% for c in pt.chanslist
				{{ c[0] }} = GP_CLAMP_GENERIC({{ c[0] }}, 0, {{ 2 ** c[2] - 1 }});
			%% endfor
			}

			GP_Pixel pix = GP_Pixel_CREATE_{{ pt.name }}({{ expand_chanslist(pt, "(uint8_t)") }});
			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, j, i, pix);
		}

		if (GP_ProgressCallbackReport(callback, i, dst->h, dst->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

%% endif
%% endfor

int GP_FilterResizeCubicInt_Raw(const GP_Context *src, GP_Context *dst,
                                GP_ProgressCallback *callback)
{
	switch (src->pixel_type) {
	%% for pt in pixeltypes
	%% if not pt.is_unknown() and not pt.is_palette()
	case GP_PIXEL_{{ pt.name }}:
		return GP_FilterResizeCubicInt_{{ pt.name }}_Raw(src, dst, callback);
	break;
	%% endif
	%% endfor
	default:
		return -1;
	}
}

%% endblock body
