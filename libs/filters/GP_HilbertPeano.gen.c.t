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

%% extends "base.c.t"

{% block descr %}Hilbert Peano dithering RGB888 -> any pixel{% endblock %}

%% block body

#include "core/GP_Core.h"
#include "core/GP_GetPutPixel.h"
#include "GP_HilbertCurve.h"
#include "GP_Filter.h"

/*
 * Returns closest greater square of two, used to determine the curve size.
 */
static unsigned int count_bits(unsigned int n)
{
	unsigned int i = 0, s = n;

	do {
		n>>=1;
		i++;
	} while (n);

	i--;

	return (i + (s != (1U<<i)));
}

%% for pt in pixeltypes
%% if not pt.is_unknown() and not pt.is_alpha() and not pt.is_palette()
/*
 * Hilbert Peano RGB888 to {{ pt.name }}
 */
int GP_FilterHilbertPeano_RGB888_to_{{ pt.name }}_Raw(const GP_Context *src,
                                                 GP_Context *dst,
                                                 GP_ProgressCallback *callback)
{
	struct GP_CurveState state;
	unsigned int n;

	n = GP_MAX(count_bits(src->w), count_bits(src->h));

	GP_DEBUG(1, "Hilbert Peano dithering %ux%u -> n = %u", src->w, src->h, n);

	GP_HilbertCurveInit(&state, n);
	
	/* processed pixels counter */
	unsigned int cnt = 0;

	/* error counters */
%% for c in pt.chanslist
	int err_{{ c[0] }} = 0;
%% endfor

	while (GP_HilbertCurveContinues(&state)) {
		if (state.x < src->w && state.y < src->h) {
			GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, state.x, state.y);

%% for c in pt.chanslist
%% if pt.is_gray()
			int pix_{{ c[0] }} = GP_Pixel_GET_R_RGB888(pix) +
			                     GP_Pixel_GET_G_RGB888(pix) +
			                     GP_Pixel_GET_B_RGB888(pix);
%% else
			int pix_{{ c[0] }} = GP_Pixel_GET_{{ c[0] }}_RGB888(pix);
%% endif			

			pix_{{ c[0] }} += err_{{ c[0] }};

%% if pt.is_gray()
			int res_{{ c[0] }} = ({{ 2 ** c[2] - 1}} * pix_{{ c[0] }} + 382) / {{ 3 * 255 }};
			err_{{ c[0] }} = pix_{{ c[0] }} - {{ 3 * 255 }} * res_{{ c[0] }} / {{ 2 ** c[2] - 1 }};
%% else
			int res_{{ c[0] }} = ({{ 2 ** c[2] - 1}} * pix_{{ c[0] }} + 127) / 255;
			err_{{ c[0] }} = pix_{{ c[0] }} - 255 * res_{{ c[0] }} / {{ 2 ** c[2] - 1 }};
%% endif
%% endfor

%% if pt.is_gray()
			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, state.x, state.y, res_V);
%% else
			GP_Pixel res = GP_Pixel_CREATE_{{ pt.name }}(res_{{ pt.chanslist[0][0] }}{% for c in pt.chanslist[1:] %}, res_{{ c[0] }}{% endfor %});

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, state.x, state.y, res);
%% endif
			cnt++;

			if (GP_ProgressCallbackReport(callback, cnt/src->h, src->w, src->h))
				return 1;

			/* We are done, exit */
			if (cnt == src->w * src->h - 1) {
				GP_ProgressCallbackDone(callback);
				return 0;
			}
		} else {
%% for c in pt.chanslist
			err_{{ c[0] }} = 0;
%% endfor
		}

		GP_HilbertCurveNext(&state);
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

%% endif
%% endfor

int GP_FilterHilbertPeano_RGB888_Raw(const GP_Context *src,
                                     GP_Context *dst,
                                     GP_ProgressCallback *callback)
{
	switch (dst->pixel_type) {
%% for pt in pixeltypes
%% if not pt.is_unknown() and not pt.is_alpha() and not pt.is_palette()
	case GP_PIXEL_{{ pt.name }}:
		return GP_FilterHilbertPeano_RGB888_to_{{ pt.name }}_Raw(src, dst, callback);
%% endif
%% endfor
	default:
		return 1;
	}
}

%% endblock body
