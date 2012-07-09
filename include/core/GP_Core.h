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

  This is header file for public core API.

 */

#ifndef GP_CORE_H
#define GP_CORE_H

/* Common building blocks */
#include "core/GP_Common.h"

/* Context ... */
#include "core/GP_Context.h"

/* ... and it's trasformations */
#include "core/GP_Transform.h"

/* Gamma */
#include "core/GP_Gamma.h"

/* Pixeltypes */
#include "core/GP_Pixel.h"

/* Pixel conversions */
#include "core/GP_Convert.h"

/* Individual pixel access */
#include "core/GP_GetPutPixel.h"

/* Writing pixel blocks */
#include "GP_WritePixel.h"

/* Blitting */
#include "core/GP_Blit.h"

/* Debug and debug level */
#include "core/GP_Debug.h"

/* Color */
#include "core/GP_Color.h"

/* Progress callback */
#include "core/GP_ProgressCallback.h"

/* Threads utils */
#include "core/GP_Threads.h"

/* Debug counters */
#include "core/GP_Counter.h"

/* Mix Pixel */
#include "core/GP_MixPixels.h"

#endif /* GP_CORE_H */
