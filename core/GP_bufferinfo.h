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

#ifndef GP_BUFFERINFO_H
#define GP_BUFFERINFO_H

#include <stdint.h>
#include <unistd.h>

/* Describes a buffer in memory where drawing takes place. */
struct GP_BufferInfo {
	void *pixels;
	uint8_t bits_per_pixel;
	uint32_t bytes_per_row;
	uint32_t rows;
	uint32_t columns;
	int rows_are_vertical:1;	/* image orientation */
};

#define GP_PIXEL_ADDRESS(buffer, row, column) ((uint8_t *) buffer->pixels \
	+ row * buffer->bytes_per_row \
	+ column * (buffer->bits_per_pixel / 8))

#endif /* GP_BUFFERINFO_H */
