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

#ifndef GP_H
#define GP_H

#include <stdint.h>

/* basic definitions and structures */
#include "GP_Common.h"
#include "GP_Transform.h"
#include "GP_Context.h"

/* semi-public, low-level drawing API */
#include "GP_WritePixel.h"

/* colors */
#include "GP_Color.h"
#include "GP_Palette.h"

/* public drawing API */
#include "GP_Fill.h"
#include "GP_GetPixel.h"
#include "GP_PutPixel.h"
#include "GP_HLine.h"
#include "GP_VLine.h"
#include "GP_Line.h"
#include "GP_LineTrack.h"
#include "GP_Rect.h"
#include "GP_FillRect.h"
#include "GP_Triangle.h"
#include "GP_FillTriangle.h"
#include "GP_Tetragon.h"
#include "GP_FillTetragon.h"
#include "GP_Circle.h"
#include "GP_FillCircle.h"
#include "GP_Ellipse.h"
#include "GP_FillEllipse.h"
#include "GP_Symbol.h"

/* fonts */
#include "GP_Font.h"
#include "GP_TextStyle.h"
#include "GP_TextMetric.h"
#include "GP_Text.h"

#endif /* GP_H */
