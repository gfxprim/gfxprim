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

#ifndef GP_HLINE_H
#define GP_HLINE_H

#include "GP_Context.h"

#include <stdint.h>

void GP_HLine1bpp(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel);
void GP_HLine2bpp(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel);
void GP_HLine8bpp(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel);
void GP_HLine16bpp(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel);
void GP_HLine24bpp(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel);
void GP_HLine32bpp(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel);

GP_RetCode GP_HLineXXY(GP_Context *context, int x0, int x1, int y,
	GP_Pixel pixel);

GP_RetCode GP_HLineXYW(GP_Context *context, int x, int y, unsigned int w,
	GP_Pixel pixel);

GP_RetCode GP_THLineXXY(GP_Context *context, int x0, int x1, int y,
	GP_Pixel pixel);

GP_RetCode GP_THLineXYW(GP_Context *context, int x, int y, unsigned int w,
	GP_Pixel pixel);

/* default argument set is XXY */
#define GP_HLine GP_HLineXXY
#define GP_THLine GP_THLineXXY

#endif /* GP_HLINE_H */
