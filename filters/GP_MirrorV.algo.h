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


#include "GP_Clip.h"

#define DEF_MIRRORV_FN(FN_NAME, CONTEXT_T, PIXEL_T, PUTPIXEL, GETPIXEL) \
void FN_NAME(CONTEXT_T context) \
{ \
	uint32_t x, y; \
	PIXEL_T tmp; \
\
	for (x = 0; x < context->w/2; x++) { \
		uint32_t xm = context->w - x - 1; \
		for (y = 0; y < context->h; y++) { \
			tmp = GETPIXEL(context, x, y); \
\
			PUTPIXEL(context, x, y, GETPIXEL(context, xm, y)); \
			PUTPIXEL(context, xm, y, tmp); \
		} \
	} \
\
	GP_MIRROR_V_CLIP(context); \
}
