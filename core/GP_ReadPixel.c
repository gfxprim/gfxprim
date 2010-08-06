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

#include "GP_ReadPixel.h"

#include <endian.h>
#include <stdint.h>
#include <unistd.h>

inline uint32_t GP_ReadPixel8bpp(void *ptr)
{
	return (uint32_t) *((uint8_t *) ptr);
}

inline uint32_t GP_ReadPixel16bpp(void *ptr)
{
	return (uint32_t) *((uint16_t *) ptr);
}

inline uint32_t GP_ReadPixel24bpp(void *ptr)
{
#if __BYTE_ORDER == __BIG_ENDIAN

	return ((uint32_t) ((uint8_t *) ptr)[0]) << 16
		| ((uint32_t) ((uint8_t *) ptr)[1]) << 8
		| ((uint32_t) ((uint8_t *) ptr)[2]);

#elif __BYTE_ORDER == __LITTLE_ENDIAN

	return ((uint32_t) ((uint8_t *) ptr)[0])
		| ((uint32_t) ((uint8_t *) ptr)[1]) << 8
		| ((uint32_t) ((uint8_t *) ptr)[2]) << 16;

#else
#error "Could not detect machine endianity"
#endif
}

inline uint32_t GP_ReadPixel32bpp(void *ptr)
{
	return *((uint32_t *) ptr);
}
