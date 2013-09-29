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

%% extends 'base.c.t'

{% block descr %}
Optimized fill functions.
{% endblock descr %}

%% block body

#include "core/GP_Context.h"
#include "core/GP_WritePixel.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_FnPerBpp.h"
#include "core/GP_Fill.h"

%% from './WritePixels.t' import optimized_writepixels

%% for ps in pixelsizes
static void fill_{{ ps.suffix }}(GP_Context *ctx, GP_Pixel val)
{
	unsigned int y;

	for (y = 0; y < ctx->h; y++) {
%%  if ps.suffix in optimized_writepixels
		void *start = GP_PIXEL_ADDR(ctx, 0, y);
%%   if ps.needs_bit_endian()
		GP_WritePixels_{{ ps.suffix }}(start, 0, ctx->w, val);
%%   else
		GP_WritePixels_{{ ps.suffix }}(start, ctx->w, val);
%%   endif
%%  else
		unsigned int x;

		for (x = 0; x < ctx->w; x++)
			GP_PutPixel_Raw_{{ ps.suffix }}(ctx, x, y, val);
%%  endif
	}
}

%% endfor

void GP_Fill(GP_Context *ctx, GP_Pixel val)
{
	GP_FN_PER_BPP_CONTEXT(fill, ctx, ctx, val);
}
%% endblock body
