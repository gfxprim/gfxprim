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

#ifndef GP_WRITEPIXEL_H
#define GP_WRITEPIXEL_H

#include <endian.h>
#include <stdint.h>
#include <unistd.h>

/*
 * Macros for writing a single pixel value to the specified address,
 * provided that the target buffer has 8, 16, 24, or 32 bytes per pixel.
 */

#define GP_WritePixel8bpp(ptr, pixel) { \
	*((uint8_t *) ptr) = (uint8_t) pixel; \
}

#define GP_WritePixel16bpp(ptr, pixel) { \
	*((uint16_t *) ptr) = (uint16_t) pixel; \
}

#if __BYTE_ORDER == __BIG_ENDIAN

#define GP_WritePixel24bpp(ptr, pixel) { \
	((uint8_t *) ptr)[0] = (pixel >> 16) & 0xff; \
	((uint8_t *) ptr)[1] = (pixel >> 8) & 0xff; \
	((uint8_t *) ptr)[2] = pixel & 0xff; \
}

#elif __BYTE_ORDER == __LITTLE_ENDIAN

#define GP_WritePixel24bpp(ptr, pixel) { \
	((uint8_t *) ptr)[0] = pixel & 0xff; \
	((uint8_t *) ptr)[1] = (pixel >> 8) & 0xff; \
	((uint8_t *) ptr)[2] = (pixel >> 16) & 0xff; \
}

#else
#error "Could not detect machine endianity"
#endif

#define GP_WritePixel32bpp(ptr, pixel) { \
	*((uint32_t *) ptr) = (uint32_t) pixel; \
}

/*
 * Calls for writing a linear block of pixels.
 */

/*
 * These calls are not byte aligned, thuss needs start offset.
 */
void GP_WritePixels1bpp(uint8_t *start, uint8_t off, size_t cnt, uint8_t val);
void GP_WritePixels2bpp(uint8_t *start, uint8_t off, size_t cnt, uint8_t val);
void GP_WritePixels4bpp(uint8_t *start, uint8_t off, size_t cnt, uint8_t val);

void GP_WritePixels8bpp(void *start, size_t count, uint8_t value);
void GP_WritePixels16bpp(void *start, size_t count, uint16_t value);
void GP_WritePixels24bpp(void *start, size_t count, uint32_t value);
void GP_WritePixels32bpp(void *start, size_t count, uint32_t value);

#endif /* GP_WRITEPIXEL_H */
