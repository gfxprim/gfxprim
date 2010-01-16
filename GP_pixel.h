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

#ifndef GP_PIXEL_H
#define GP_PIXEL_H

#include "GP_backend.h"
#include "GP_writepixel.h"

/*
 * Common API functions for getting/setting pixels.
 */

/*
 * Set of arguments passed to the GP_SetPixel() routine and similar ones.
 */
#define SETPIXEL_ARGS GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x, int y

/*
 * Sets a pixel at coordinates (x, y) to the given color.
 * Honors clipping and edges: if the coordinates lie outside the clipping
 * rectangle or the target buffer, the call is a safe no-op.
 */
void GP_SetPixel(GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x, int y);

/*
 * Reads a pixel at the specified coordinates, returning its color.
 * This call is not affected by the clipping rectangle, but still honors
 * target buffer boundary; if the coordinates lie outside the buffer,
 * result is 0.
 */
GP_COLOR_TYPE GP_GetPixel(GP_TARGET_TYPE *target, int x, int y);

/* commonly used alternative name */
#define GP_PutPixel GP_SetPixel

/*
 * Variants of SetPixel() used when we know the bit depth of the surface,
 * and want to draw many pixels at once; this saves the check at every call.
 * These variants also respect clipping and are overdraw-safe.
 */

void GP_SetPixel_8bpp(SETPIXEL_ARGS);
void GP_SetPixel_16bpp(SETPIXEL_ARGS);
void GP_SetPixel_24bpp(SETPIXEL_ARGS);
void GP_SetPixel_32bpp(SETPIXEL_ARGS);

/*
 * Unsafe variants that do not check clipping nor target boundary.
 */
void GP_SetPixel_8bpp_unsafe(SETPIXEL_ARGS);
void GP_SetPixel_16bpp_unsafe(SETPIXEL_ARGS);
void GP_SetPixel_24bpp_unsafe(SETPIXEL_ARGS);
void GP_SetPixel_32bpp_unsafe(SETPIXEL_ARGS);

#endif /* GP_PIXEL_H */

