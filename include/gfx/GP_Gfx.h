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

  This is a main header for gfx part.

 */

#ifndef GP_GFX_H
#define GP_GFX_H

/* basic definitions and structures */
#include "core/GP_Common.h"
#include "core/GP_Transform.h"
#include "core/GP_Context.h"
#include "core/GP_WritePixel.h"
#include "core/GP_GetPixel.h"
#include "core/GP_PutPixel.h"
#include "core/GP_Color.h"

/* public drawing API */
#include "GP_Fill.h"
#include "GP_HLine.h"
#include "GP_VLine.h"
#include "GP_Line.h"
#include "GP_Rect.h"
#include "GP_Triangle.h"
#include "GP_Tetragon.h"
#include "GP_Circle.h"
#include "GP_Ellipse.h"
#include "GP_Polygon.h"
#include "GP_Symbol.h"

#endif /* GP_GFX_H */
