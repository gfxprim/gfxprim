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
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*
  * Draws a symbol into rectangle.
  */

#ifndef GP_SYMBOL_H
#define GP_SYMBOL_H

#include "core/GP_Types.h"

typedef enum GP_SymbolType {
	GP_SYM_TRIANGLE_UP,
	GP_SYM_TRIANGLE_DOWN,
	GP_SYM_TRIANGLE_LEFT,
	GP_SYM_TRIANGLE_RIGHT,
	GP_SYM_MAX,
} GP_SymbolType;

/* Symbol */

void GP_Symbol(GP_Context *context, GP_SymbolType sym,
               GP_Coord x, GP_Coord y,
               GP_Size w, GP_Size h, GP_Pixel pixel);

void GP_Symbol_Raw(GP_Context *context, GP_SymbolType sym,
                   GP_Coord x, GP_Coord y,
                   GP_Size w, GP_Size h, GP_Pixel pixel);

/* Filled Symbol */

void GP_FillSymbol(GP_Context *context, GP_SymbolType sym,
                   GP_Coord x, GP_Coord y,
		   GP_Size w, GP_Size h, GP_Pixel pixel);

void GP_FillSymbol_Raw(GP_Context *context, GP_SymbolType sym,
                       GP_Coord x, GP_Coord y,
		       GP_Size w, GP_Size h, GP_Pixel pixel);

#endif /* GP_SYMBOL_H */
