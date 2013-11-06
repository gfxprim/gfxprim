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
#include "core/GP_Debug.h"

#include "filters/GP_ApplyTables.h"

%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()
static int apply_tables_{{ pt.name }}(const GP_FilterArea *const area,
                                      const GP_FilterTables *const tables,
                                      GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Point filter %ux%u", area->src_w, area->src_h);

	unsigned int x, y;
	const GP_Context *src = area->src;
	GP_Context *dst = area->dst;

%%   for c in pt.chanslist
	GP_Pixel {{ c.name }};
%%   endfor

	for (y = 0; y < area->src_h; y++) {
		for (x = 0; x < area->src_w; x++) {
			unsigned int src_x = area->src_x + x;
			unsigned int src_y = area->src_y + y;
			unsigned int dst_x = area->dst_x + x;
			unsigned int dst_y = area->dst_y + y;

			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, src_x, src_y);

%%   for c in pt.chanslist
			{{ c.name }} = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix);
			{{ c.name }} = tables->table[{{ c.idx }}][{{ c.name }}];
%%    endfor

			pix = GP_Pixel_CREATE_{{ pt.name }}({{ expand_chanslist(pt, "", "") }});
			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, dst_x, dst_y, pix);
		}

		if (GP_ProgressCallbackReport(callback, y, area->src_h, area->src_w)) {
			errno = ECANCELED;
			return 1;
		}
	}

	GP_ProgressCallbackDone(callback);

	return 0;
}

%%  endif
%% endfor

int GP_FilterTablesApply(const GP_FilterArea *const area,
                         const GP_FilterTables *const tables,
                         GP_ProgressCallback *callback)
{
	GP_ASSERT(area->src->pixel_type == area->dst->pixel_type);
	//TODO: Assert size

	switch (area->src->pixel_type) {
%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()
	case GP_PIXEL_{{ pt.name }}:
		return apply_tables_{{ pt.name }}(area, tables, callback);
	break;
%%  endif
%% endfor
	default:
		errno = EINVAL;
		return -1;
	}
}

%% endblock body
