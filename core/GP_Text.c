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

#define CONTEXT_T GP_Context *
#define PIXVAL_T GP_Color
	#define HLINE GP_HLine
	#define FN_NAME GP_Text_internal
		#include "algo/Text.algo.c"
	#undef HLINE
	#undef FN_NAME
	#define HLINE GP_THLine
	#define FN_NAME GP_TText_internal
		#include "algo/Text.algo.c"
	#undef FN_NAME
	#undef HLINE
#undef PIXVAL_T
#undef CONTEXT_T

GP_RetCode GP_Text(GP_Context *context, const GP_TextStyle *style,
	int x, int y, const char *str, GP_Color color)
{
	GP_Text_internal(context, style, x, y, str, color);
	return GP_ESUCCESS;
}

GP_RetCode GP_TText(GP_Context *context, const GP_TextStyle *style,
	int x, int y, const char *str, GP_Color color)
{
	GP_TText_internal(context, style, x, y, str, color);
	return GP_ESUCCESS;
}

