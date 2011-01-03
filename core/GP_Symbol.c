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

#include "GP.h"

#define DO_TRIANGLE_UP(x, y, w, h) \
	x,     y + h, x + w, y + h, x + w/2, y

#define DO_TRIANGLE_DOWN(x, y, w, h) \
	x,     y,     x + w, y,     x + w/2, y + h

#define DO_TRIANGLE_LEFT(x, y, w, h) \
	x + w, y,     x + w, y + h, x,       y + h/2

#define DO_TRIANGLE_RIGHT(x, y, w, h) \
	x,     y,     x,     y + h, x + w,   y + h/2

#define DO_DECREMENT(a) do {        \
	if ((a) == 0)               \
		return GP_ESUCCESS; \
	(a)--;                      \
} while (0)

/*
 * TODO: even sizes should have two pixes at the top to became symetrical
 */
GP_RetCode GP_Symbol(GP_Context *context, GP_SymbolType sym,
                     int x, int y, int w, int h,
                     GP_Pixel pixel)
{
	DO_DECREMENT(w);
	DO_DECREMENT(h);

	switch (sym) {
	case GP_SYM_TRIANGLE_UP:
		return GP_Triangle(context, DO_TRIANGLE_UP(x, y, w, h), 
		                   pixel);
	case GP_SYM_TRIANGLE_DOWN:
		return GP_Triangle(context, DO_TRIANGLE_DOWN(x, y, w, h),
		                   pixel);
	case GP_SYM_TRIANGLE_LEFT:
		return GP_Triangle(context, DO_TRIANGLE_LEFT(x, y, w, h),
		                   pixel);
	case GP_SYM_TRIANGLE_RIGHT:
		return GP_Triangle(context, DO_TRIANGLE_RIGHT(x, y, w, h), 
		                   pixel);
	default:
		return GP_ENOIMPL;
	}
}

GP_RetCode GP_FillSymbol(GP_Context *context, GP_SymbolType sym,
                         int x, int y, int w, int h,
                         GP_Pixel pixel)
{
	DO_DECREMENT(w);
	DO_DECREMENT(h);
	
	switch (sym) {
	case GP_SYM_TRIANGLE_UP:
		return GP_FillTriangle(context, DO_TRIANGLE_UP(x, y, w, h), 
		                       pixel);
	case GP_SYM_TRIANGLE_DOWN:
		return GP_FillTriangle(context, DO_TRIANGLE_DOWN(x, y, w, h), 
		                       pixel);
	case GP_SYM_TRIANGLE_LEFT:
		return GP_FillTriangle(context, DO_TRIANGLE_DOWN(x, y, w, h),
		                       pixel);
	case GP_SYM_TRIANGLE_RIGHT:
		return GP_FillTriangle(context, DO_TRIANGLE_RIGHT(x, y, w, h), 
		                       pixel);
	default:
		return GP_ENOIMPL;
	}
}

GP_RetCode GP_TSymbol(GP_Context *context, GP_SymbolType sym,
                      int x, int y, int w, int h,
                      GP_Pixel pixel)
{
	DO_DECREMENT(w);
	DO_DECREMENT(h);
	
	switch (sym) {
	case GP_SYM_TRIANGLE_UP:
		return GP_TTriangle(context, DO_TRIANGLE_UP(x, y, w, h), 
		                    pixel);
	case GP_SYM_TRIANGLE_DOWN:
		return GP_TTriangle(context, DO_TRIANGLE_DOWN(x, y, w, h),
		                    pixel);
	case GP_SYM_TRIANGLE_LEFT:
		return GP_TTriangle(context, DO_TRIANGLE_LEFT(x, y, w, h),
		                    pixel);
	case GP_SYM_TRIANGLE_RIGHT:
		return GP_TTriangle(context, DO_TRIANGLE_RIGHT(x, y, w, h), 
		                    pixel);
	default:
		return GP_ENOIMPL;
	}
}

GP_RetCode GP_TFillSymbol(GP_Context *context, GP_SymbolType sym,
                          int x, int y, int w, int h,
                      GP_Pixel pixel)
{
	DO_DECREMENT(w);
	DO_DECREMENT(h);
	
	switch (sym) {
	case GP_SYM_TRIANGLE_UP:
		return GP_TFillTriangle(context, DO_TRIANGLE_UP(x, y, w, h), 
		                        pixel);
	case GP_SYM_TRIANGLE_DOWN:
		return GP_TFillTriangle(context, DO_TRIANGLE_DOWN(x, y, w, h), 
		                        pixel);
	case GP_SYM_TRIANGLE_LEFT:
		return GP_TFillTriangle(context, DO_TRIANGLE_DOWN(x, y, w, h),
		                        pixel);
	case GP_SYM_TRIANGLE_RIGHT:
		return GP_TFillTriangle(context, DO_TRIANGLE_RIGHT(x, y, w, h), 
		                        pixel);
	default:
		return GP_ENOIMPL;
	}
}
