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

{% block descr %}Vertical Mirror alogorithm{% endblock %}

%% block body

#include "core/GP_GetPutPixel.h"
#include "core/GP_Debug.h"
#include "GP_Rotate.h"

%% for ps in pixelsizes
static int GP_MirrorV_Raw_{{ ps.suffix }}(const GP_Context *src,
                                    GP_Context *dst,
                                    GP_ProgressCallback *callback)
{
	uint32_t x, y;
	GP_Pixel tmp;

	GP_DEBUG(1, "Mirroring image vertically %ux%u", src->w, src->h);
	
	for (x = 0; x < src->w/2; x++) {
		uint32_t xm = src->w - x - 1;
		for (y = 0; y < src->h; y++) {
			tmp = GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y);
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, x, y, GP_GetPixel_Raw_{{ ps.suffix }}(src, xm, y));
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, xm, y, tmp);
		}

		if (GP_ProgressCallbackReport(callback, 2 * x, src->w, src->h))
			return 1;
	}

	/* Copy the middle odd line */
	if (src != dst && src->w % 2) {
		x = src->w / 2;
		for (y = 0; y < src->h; y++)
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, x, y, GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y));
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

%% endfor

int GP_FilterMirrorV_Raw(const GP_Context *src, GP_Context *dst,
                          GP_ProgressCallback *callback)
{
	GP_FN_RET_PER_BPP_CONTEXT(GP_MirrorV_Raw, src, src, dst, callback);
	return 1;
}

%% endblock body
