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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 * Copyright (C) 2011      Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*
  
   The macros are taking generally three arguments
  
  */

#ifndef CORE_GP_GET_SET_BITS_H
#define CORE_GP_GET_SET_BITS_H

/*
 * Helper macros to read/write parts of words 
 *
 * Return (shifted) count bits at offset of value
 * Note: operates with value types same as val 
 */
#define GP_GET_BITS(offset, len, val) \
	( ( (val)>>(offset) ) & ( ((((typeof(val))1)<<(len)) - 1) ) )

/*
 * Align-safe getbits
 *
 * TODO: Fix big endian
 */
#define GP_GET_BITS4_ALIGNED(offset, len, val) ({ \
	uint32_t v;                               \
	v  = ((uint8_t *)&val)[0];                \
	v |= ((uint8_t *)&val)[1]<<8;             \
	v |= ((uint8_t *)&val)[2]<<16;            \
	v |= ((uint8_t *)&val)[3]<<24;            \
                                                  \
	GP_GET_BITS(offset, len, v);              \
})

#define GP_GET_BITS3_ALIGNED(offset, len, val) ({ \
	uint32_t v;                               \
	v  = ((uint8_t *)&val)[0];                \
	v |= ((uint8_t *)&val)[1]<<8;             \
	v |= ((uint8_t *)&val)[2]<<16;            \
                                                  \
	GP_GET_BITS(offset, len, v);              \
})

#define GP_GET_BITS2_ALIGNED(offset, len, val) ({ \
	uint16_t v;                               \
	v  = ((uint8_t *)&val)[0];                \
	v |= ((uint8_t *)&val)[1]<<8;             \
                                                  \
	GP_GET_BITS(offset, len, v);              \
})

#define GP_GET_BITS1_ALIGNED(offset, len, val) ({ \
	uint8_t v;                                \
	v = ((uint8_t *)&val)[0];                 \
                                                  \
	GP_GET_BITS(offset, len, v);              \
})

/*
 * Set count bits of dest at ofset to val (shifted by offset)
 * 
 * Does not check val for overflow
 * Operates on 8, 16, and 32 bit values, depending on the type of dest, 
 * this should be unsigned
 *
 * GP_SET_BITS_OR anly sets (|=) the bits, assuming these are clear beforehand
 * GP_CLEAR_BITS sets the target bits to zero
 * GP_SET_BITS does both
 */
#define GP_CLEAR_BITS(offset, len, dest) \
       ( (dest) &= ~(((((typeof(dest))1) << (len)) - 1) << (offset)) )

#define GP_SET_BITS_OR(offset, dest, val) ( (dest) |= ((val)<<(offset)) )

#define GP_SET_BITS(offset, len, dest, val) do {  \
               GP_CLEAR_BITS(offset, len, dest);  \
               GP_SET_BITS_OR(offset, dest, val); \
} while (0)

/*
 * Align-safe setbits
 */
#define GP_SET_BITS1_ALIGNED(offset, len, dest, val) do { \
	uint8_t v = ((uint8_t *)dest)[0];                 \
	GP_SET_BITS(offset, len, v, val);                 \
	((uint8_t *)dest)[0] = v;                         \
} while (0)

#define GP_SET_BITS2_ALIGNED(offset, len, dest, val) do { \
	uint16_t v;                                       \
	v  = ((uint8_t *)dest)[0];                        \
	v |= ((uint8_t *)dest)[1]<<8;                     \
	                                                  \
	GP_SET_BITS(offset, len, v, val);                 \
	                                                  \
	((uint8_t *)dest)[0] = 0xff & v;                  \
	((uint8_t *)dest)[1] = 0xff & (v >> 8);           \
} while (0)

#define GP_SET_BITS3_ALIGNED(offset, len, dest, val) do { \
	uint32_t v;                                       \
	v  = ((uint8_t *)dest)[0];                        \
	v |= ((uint8_t *)dest)[1]<<8;                     \
	v |= ((uint8_t *)dest)[2]<<16;                    \
	                                                  \
	GP_SET_BITS(offset, len, v, val);                 \
	                                                  \
	((uint8_t *)dest)[0] = 0xff & v;                  \
	((uint8_t *)dest)[1] = 0xff & (v >> 8);           \
	((uint8_t *)dest)[2] = 0xff & (v >> 16);          \
} while (0)

#define GP_SET_BITS4_ALIGNED(offset, len, dest, val) do { \
	uint32_t v;                                       \
	v  = ((uint8_t *)dest)[0];                        \
	v |= ((uint8_t *)dest)[2]<<16;                    \
	v |= ((uint8_t *)dest)[3]<<24;                    \
	                                                  \
	GP_SET_BITS(offset, len, v, val);                 \
	                                                  \
	((uint8_t *)dest)[0] = 0xff & v;                  \
	((uint8_t *)dest)[1] = 0xff & (v >> 8);           \
	((uint8_t *)dest)[2] = 0xff & (v >> 16);          \
	((uint8_t *)dest)[3] = 0xff & (v >> 24);          \
} while (0)


#endif /* CORE_GP_GET_SET_BITS_H */
