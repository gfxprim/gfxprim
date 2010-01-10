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
 * File to be #included from setpixel.c.
 * 
 * This file is a macro template for building specialized SetPixel() functions
 * for various bit depths. It can be included multiple times; each inclusion
 * will generate a definition of a function.
 *
 * These arguments must be #defined in the including file:
 *
 *	FN_NAME
 *		The name you wish for the newly defined function.
 *
 *	WRITE_PIXEL
 *		A routine to write a pixel value to a memory address;
 *		must have form: void WRITE_PIXEL(uint8_t *p, long color)
 */

void FN_NAME(GP_TARGET_TYPE *target, long color, int x, int y)
{
	if (target == NULL || GP_PIXELS(target) == NULL)
		return;

	int xmin, xmax, ymin, ymax;
	GP_GET_CLIP_RECT(target, xmin, xmax, ymin, ymax);

	if (x < xmin || y < ymin || x > xmax || y > ymax)
		return;

	uint8_t *p = GP_PIXEL_ADDR(target, x, y);
	WRITE_PIXEL(p, color);
}

