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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "core/GP_Clip.h"
#include "core/GP_Common.h"
#include "core/GP_Context.h"

#define DEF_ROTATECW_FN(FN_NAME, CONTEXT_T, PIXEL_T, PUTPIXEL, GETPIXEL) \
GP_RetCode FN_NAME(CONTEXT_T context) \
{ \
	uint32_t x, y; \
	CONTEXT_T tmp; \
\
	tmp = GP_ContextCopy(context, GP_COPY_WITH_PIXELS); \
\
	if (tmp == NULL) \
		return GP_ENOMEM; \
\
	GP_SWAP(context->w, context->h); \
\
	context->bytes_per_row = GP_CALC_ROW_SIZE(context->pixel_type, \
	                                          context->w); \
\
	for (x = 0; x < tmp->w; x++) { \
		for (y = 0; y < tmp->h; y++) { \
			uint32_t yr = tmp->h - y - 1; \
			PUTPIXEL(context, yr, x, GETPIXEL(tmp, x, y)); \
		} \
	} \
\
	GP_ContextFree(tmp); \
\
	GP_SWAP_CLIPS(context); \
	GP_MIRROR_H_CLIP(context); \
\
	return GP_ESUCCESS; \
}

#define DEF_ROTATECCW_FN(FN_NAME, CONTEXT_T, PIXEL_T, PUTPIXEL, GETPIXEL) \
GP_RetCode FN_NAME(CONTEXT_T context) \
{ \
	uint32_t x, y; \
	CONTEXT_T tmp; \
\
	tmp = GP_ContextCopy(context, GP_COPY_WITH_PIXELS); \
\
	if (tmp == NULL) \
		return GP_ENOMEM; \
\
	GP_SWAP(context->w, context->h); \
\
	context->bytes_per_row = GP_CALC_ROW_SIZE(context->pixel_type, \
	                                          context->w); \
\
	for (x = 0; x < tmp->w; x++) { \
		for (y = 0; y < tmp->h; y++) { \
				uint32_t xr = tmp->w - x - 1; \
				PUTPIXEL(context, y, xr, GETPIXEL(tmp, x, y)); \
		} \
	} \
\
	GP_ContextFree(tmp); \
\
	GP_SWAP_CLIPS(context); \
	GP_MIRROR_V_CLIP(context); \
\
	return GP_ESUCCESS; \
}
