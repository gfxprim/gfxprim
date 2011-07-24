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

#include "GP_Gfx.h"

#define DO_TRIANGLE_UP(x, y, w, h) \
	x,     y + h, x + w, y + h, x + w/2, y

#define DO_TRIANGLE_DOWN(x, y, w, h) \
	x,     y,     x + w, y,     x + w/2, y + h


#define DO_TRIANGLE_LEFT(x, y, w, h) \
	x + w, y,     x + w, y + h, x,       y + h/2


#define DO_TRIANGLE_RIGHT(x, y, w, h) \
	x,     y,     x,     y + h, x + w,   y + h/2

#define DO_TETRAGON_UP(x, y, w, h) \
	DO_TRIANGLE_UP(x, y, w, h),    x + w/2 + 1, y

#define DO_TETRAGON_DOWN(x, y, w, h) \
	DO_TRIANGLE_DOWN(x, y, w, h),  x + w/2 + 1, y + h

#define DO_TETRAGON_LEFT(x, y, w, h) \
	DO_TRIANGLE_LEFT(x, y, w, h),  x          , y + h/2 + 1

#define DO_TETRAGON_RIGHT(x, y, w, h) \
	DO_TRIANGLE_RIGHT(x, y, w, h), x + w      , y + h/2 + 1

#define DO_DECREMENT(a) do {        \
	if ((a) == 0)               \
		return;             \
	(a)--;                      \
} while (0)

/*
 * Generate code for triangle
 */
#define TRIANGLE(context, base, x, y, w, h, fn_pref,                           \
                 TRIANGLE_PARAMS, TETRAGON_PARAMS, pixel) do {                 \
	if (base % 2)                                                          \
		fn_pref##Tetragon(context, TETRAGON_PARAMS(x, y, w, h), pixel);\
	else                                                                   \
		fn_pref##Triangle(context, TRIANGLE_PARAMS(x, y, w, h), pixel);\
} while (0)

#define TRIANGLE_RAW(context, base, x, y, w, h, fn_pref,                           \
                     TRIANGLE_PARAMS, TETRAGON_PARAMS, pixel) do {                 \
	if (base % 2)                                                              \
		fn_pref##Tetragon_Raw(context, TETRAGON_PARAMS(x, y, w, h), pixel);\
	else                                                                       \
		fn_pref##Triangle_Raw(context, TRIANGLE_PARAMS(x, y, w, h), pixel);\
} while (0)

void GP_Symbol_Raw(GP_Context *context, GP_SymbolType sym,
                   GP_Coord x, GP_Coord y, GP_Size w, GP_Size h,
                   GP_Pixel pixel)
{
	DO_DECREMENT(w);
	DO_DECREMENT(h);

	switch (sym) {
	case GP_SYM_TRIANGLE_UP:
		TRIANGLE_RAW(context, w, x, y, w, h, GP_,
		             DO_TRIANGLE_UP, DO_TETRAGON_UP, pixel);
	break;
	case GP_SYM_TRIANGLE_DOWN:
		TRIANGLE_RAW(context, w, x, y, w, h, GP_,
		             DO_TRIANGLE_DOWN, DO_TETRAGON_DOWN, pixel);
	break;
	case GP_SYM_TRIANGLE_LEFT:
		TRIANGLE_RAW(context, h, x, y, w, h, GP_,
		             DO_TRIANGLE_LEFT, DO_TETRAGON_LEFT, pixel);
	break;
	case GP_SYM_TRIANGLE_RIGHT:
		TRIANGLE_RAW(context, h, x, y, w, h, GP_,
		             DO_TRIANGLE_RIGHT, DO_TETRAGON_RIGHT, pixel);
	break;
	default:
	break;
	}
}

void GP_FillSymbol_Raw(GP_Context *context, GP_SymbolType sym,
                       GP_Coord x, GP_Coord y, GP_Size w, GP_Size h,
                       GP_Pixel pixel)
{
	DO_DECREMENT(w);
	DO_DECREMENT(h);
	
	switch (sym) {
	case GP_SYM_TRIANGLE_UP:
		TRIANGLE_RAW(context, w, x, y, w, h, GP_Fill,
		             DO_TRIANGLE_UP, DO_TETRAGON_UP, pixel);
	break;
	case GP_SYM_TRIANGLE_DOWN:
		TRIANGLE_RAW(context, w, x, y, w, h, GP_Fill,
		             DO_TRIANGLE_DOWN, DO_TETRAGON_DOWN, pixel);
	break;
	case GP_SYM_TRIANGLE_LEFT:
		TRIANGLE_RAW(context, h, x, y, w, h, GP_Fill,
		             DO_TRIANGLE_LEFT, DO_TETRAGON_LEFT, pixel);
	break;
	case GP_SYM_TRIANGLE_RIGHT:
		TRIANGLE_RAW(context, h, x, y, w, h, GP_Fill,
		             DO_TRIANGLE_RIGHT, DO_TETRAGON_RIGHT, pixel);
	break;
	default:
	break;
	}
}

void GP_Symbol(GP_Context *context, GP_SymbolType sym,
               GP_Coord x, GP_Coord y, GP_Size w, GP_Size h,
               GP_Pixel pixel)
{
	DO_DECREMENT(w);
	DO_DECREMENT(h);
	
	switch (sym) {
	case GP_SYM_TRIANGLE_UP:
		TRIANGLE(context, w, x, y, w, h, GP_,
		         DO_TRIANGLE_UP, DO_TETRAGON_UP, pixel);
	break;
	case GP_SYM_TRIANGLE_DOWN:
		TRIANGLE(context, w, x, y, w, h, GP_,
		         DO_TRIANGLE_DOWN, DO_TETRAGON_DOWN, pixel);
	break;
	case GP_SYM_TRIANGLE_LEFT:
		TRIANGLE(context, h, x, y, w, h, GP_,
		         DO_TRIANGLE_LEFT, DO_TETRAGON_LEFT, pixel);
	break;
	case GP_SYM_TRIANGLE_RIGHT:
		TRIANGLE(context, h, x, y, w, h, GP_,
		         DO_TRIANGLE_RIGHT, DO_TETRAGON_RIGHT, pixel);
	break;
	default:
	break;
	}
}

void GP_FillSymbol(GP_Context *context, GP_SymbolType sym,
                   GP_Coord x, GP_Coord y,
                   GP_Size w, GP_Size h, GP_Pixel pixel)
{
	DO_DECREMENT(w);
	DO_DECREMENT(h);
	
	switch (sym) {
	case GP_SYM_TRIANGLE_UP:
		TRIANGLE(context, w, x, y, w, h, GP_Fill,
		         DO_TRIANGLE_UP, DO_TETRAGON_UP, pixel);
	break;
	case GP_SYM_TRIANGLE_DOWN:
		TRIANGLE(context, w, x, y, w, h, GP_Fill,
		         DO_TRIANGLE_DOWN, DO_TETRAGON_DOWN, pixel);
	break;
	case GP_SYM_TRIANGLE_LEFT:
		TRIANGLE(context, h, x, y, w, h, GP_Fill,
		         DO_TRIANGLE_LEFT, DO_TETRAGON_LEFT, pixel);
	break;
	case GP_SYM_TRIANGLE_RIGHT:
		TRIANGLE(context, h, x, y, w, h, GP_Fill,
		         DO_TRIANGLE_RIGHT, DO_TETRAGON_RIGHT, pixel);
	break;
	default:
	break;
	}
}
