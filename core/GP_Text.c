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
#include "algo/Text.algo.h"
#include "GP_FnPerBpp.h"

DEF_TEXT_FN(GP_Text8bpp, GP_Context *, GP_Pixel, GP_HLine8bpp)
DEF_TEXT_FN(GP_Text16bpp, GP_Context *, GP_Pixel, GP_HLine16bpp)
DEF_TEXT_FN(GP_Text24bpp, GP_Context *, GP_Pixel, GP_HLine24bpp)
DEF_TEXT_FN(GP_Text32bpp, GP_Context *, GP_Pixel, GP_HLine32bpp)
DEF_TEXT_FN(GP_TText_internal, GP_Context *, GP_Pixel, GP_THLine)

GP_RetCode GP_Text(GP_Context *context, const GP_TextStyle *style,
	int x, int y, const char *str, GP_Pixel pixel)
{
	if (!context || !style || !style->font || !str)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_FN_PER_BPP(GP_Text, style, x, y, str, pixel);

	return GP_ESUCCESS;
}

GP_RetCode GP_TText(GP_Context *context, const GP_TextStyle *style,
	int x, int y, const char *str, GP_Pixel pixel)
{
	if (!context || !style || !style->font || !str)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_TText_internal(context, style, x, y, str, pixel);
	return GP_ESUCCESS;
}

