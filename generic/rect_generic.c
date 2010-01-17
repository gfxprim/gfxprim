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

/*
 * Parameterized template for a rectangle drawing function.
 * Parameters that must be #defined outside:
 *
 * 	FN_ATTR
 * 		(Optional.) Attributes of the function (e.g. "static").
 * 	FN_NAME
 * 		Name of the function to define.
 * 	HLINE
 * 		Name of a horizontal line drawing routine,
 * 		as in hline_generic.c.
 * 	VLINE
 * 		Name of a vertical line drawing routine,
 * 		as in vline_generic.c.
 */

extern void HLINE(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int x0, int x1, int y);
extern void VLINE(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int x, int y0, int y1);

#ifndef FN_ATTR
#define FN_ATTR
#endif

FN_ATTR void FN_NAME(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int x0, int y0, int x1, int y1)
{
	HLINE(target, color, x0, x1, y0);
	HLINE(target, color, x0, x1, y1);
	VLINE(target, color, x0, y0, y1);
	VLINE(target, color, x1, y0, y1);
}

#undef FN_ATTR
#undef FN_NAME
#undef HLINE
#undef VLINE

