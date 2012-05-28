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
 * Copyright (C) 2012      Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*
  
   Inline functions for swapping bits inside of the byte

  */

#ifndef CORE_GP_BIT_SWAP_H
#define CORE_GP_BIT_SWAP_H

/*
 * Reverse 1 bit blocks in the byte.
 *
 * Example:
 *
 * IN  10101110
 * OUT 01110101
 */
static inline uint8_t GP_BIT_SWAP_B1(uint8_t byte)
{
	return ((byte * 0x0802LU & 0x22110LU) |
                (byte * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
}

static inline void GP_BitSwapRow_B1(uint8_t *row, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		row[i] = GP_BIT_SWAP_B1(row[i]);
}

/*
 * Reverse 2 bit blocks in the byte.
 *
 * Example:
 *
 * IN  10 11 01 00
 * OUT 00 01 11 10
 */
static inline uint8_t GP_BIT_SWAP_B2(uint8_t byte)
{
	return ((byte & 0xC0) >> 6) | ((byte & 0x30) >> 2) |
	       ((byte & 0x0C) << 2) | ((byte & 0x03) << 6);
}

static inline void GP_BitSwapRow_B2(uint8_t *row, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		row[i] = GP_BIT_SWAP_B2(row[i]);
}

/*
 * Reverse 4 bit blocks in the byte.
 *
 * Example:
 *
 * IN  1011 0100
 * OUT 0100 1011
 */
static inline uint8_t GP_BIT_SWAP_B4(uint8_t byte)
{
	return ((byte & 0xf0) >> 4) | ((byte & 0x0f) << 4);
}

static inline void GP_BitSwapRow_B4(uint8_t *row, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		row[i] = GP_BIT_SWAP_B4(row[i]);
}

#endif /* CORE_GP_BIT_SWAP_H */
