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

/*
 * Macros for writing a single pixel value to the specified address,
 * provided that the pixel has 1, 2, 3, or 4 bytes, respectively.
 */

#define GP_WRITE_PIXEL_1BYTE(ptr, pixel) { \
	*((uint8_t *) ptr) = (uint8_t) pixel; \
}

#define GP_WRITE_PIXEL_2BYTES(ptr, pixel) { \
	*((uint16_t *) ptr) = (uint16_t) pixel; \
}

#define GP_WRITE_PIXEL_3BYTES(ptr, pixel) { \
	if (SDL_BYTEORDER == SDL_BIG_ENDIAN) { \
		ptr[0] = (color >> 16) & 0xff; \
		ptr[1] = (color >> 8) & 0xff; \
		ptr[2] = color & 0xff; \
	} else { \
		ptr[0] = color & 0xff; \
		ptr[1] = (color >> 8) & 0xff; \
		ptr[2] = (color >> 16) & 0xff; \
	} \
}

#define GP_WRITE_PIXEL_4BYTES(ptr, pixel) { \
	*((uint32_t *) ptr) = (uint32_t) pixel; \
}

/*
 * Common API functions for getting/setting pixels,
 * with clipping and safe behavior on edges
 */

/*
 * Sets a pixel at coordinates (x, y) to the given color.
 * If the coordinates lie outside the surface boundaries or outside its
 * clipping rectangle, the call has no effect.
 */
void GP_SetPixel(GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x, int y);
long GP_GetPixel(GP_TARGET_TYPE *target, int x, int y);

/* commonly used alternative name */
#define GP_PutPixel GP_SetPixel

/*
 * Variants of SetPixel() used when we know the bit depth of the surface,
 * and want to draw many pixels at once; this saves the check at every call.
 * These variants also respect clipping and are overdraw-safe.
 */

void GP_SetPixel_8bpp(GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x, int y);
void GP_SetPixel_16bpp(GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x, int y);
void GP_SetPixel_24bpp(GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x, int y);
void GP_SetPixel_32bpp(GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x, int y);

#endif /* GP_PIXEL_H */

