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

#ifndef GP_VLINE_H
#define GP_VLINE_H

#include "GP_Context.h"

GP_RetCode GP_VLineXYY(GP_Context *context, int x, int y0, int y1,
	GP_Pixel pixel);

GP_RetCode GP_VLineXYH(GP_Context *context, int x, int y,
	unsigned int height, GP_Pixel pixel);

GP_RetCode GP_TVLineXYY(GP_Context *context, int x, int y0, int y1,
                     GP_Pixel pixel);

GP_RetCode GP_TVLineXYH(GP_Context *context, int x, int y,
	unsigned int height, GP_Pixel pixel);

/* default argument set is XYY */
#define GP_VLine GP_VLineXYY
#define GP_TVLine GP_TVLineXYY

#endif /* GP_VLINE_H */
