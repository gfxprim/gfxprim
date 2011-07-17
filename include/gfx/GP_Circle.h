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

#ifndef GP_CIRCLE_H
#define GP_CIRCLE_H

#include "core/GP_Context.h"

void GP_Circle(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
               GP_Size r, GP_Pixel pixel);

void GP_TCircle(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                GP_Size r, GP_Pixel pixel);

void GP_FillCircle(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                   GP_Size r, GP_Pixel pixel);

void GP_TFillCircle(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                    GP_Size r, GP_Pixel pixel);

void GP_FillRing(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                   GP_Size r1, GP_Size r2, GP_Pixel pixel);

void GP_TFillRing(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                    GP_Size r1, GP_Size r2, GP_Pixel pixel);

#endif /* GP_CIRCLE_H */
