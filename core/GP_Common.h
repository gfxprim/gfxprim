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

#ifndef GP_COMMON_H
#define GP_COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Returns a minimum of the two numbers.
 */

#define GP_MIN(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	_a < _b ? _a : _b; \
})

/*
 * Returns a maximum of the two numbers.
 */

#define GP_MAX(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	_a > _b ? _a : _b; \
})

/*
 * Abort and print abort location to stderr
 */

#define GP_ABORT(msg) do { \
		fprintf(stderr, "*** gfxprim: aborted: %s:%d: in %s: %s\n", \
				__FILE__, __LINE__, __FUNCTION__, #msg); \
		abort(); \
	} while (0)

/*
 * Perform a runtime check, on failure abort and print a message
 */

#define GP_CHECK(cond) do { \
		if (!(cond)) { \
			fprintf(stderr, "*** gfxprim: check failed: %s:%d: in %s: %s\n", \
				__FILE__, __LINE__, __FUNCTION__, #cond); \
			abort(); \
		} \
	} while (0)

/*
 * The standard likely() and unlikely() used in Kernel
 * TODO: Define as no-op for non-GCC compilers
 */

#ifndef likely
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#endif

/*
 * Swap a and b using an intermediate variable
 */

#define GP_SWAP(a, b) do { \
	typeof(a) tmp = b; \
	b = a;             \
	a = tmp;           \
} while (0)

/*
 * Helper macros to read/write parts of words 
 *
 * Return (shifted) count bits at offset of value
 * Note: operates with value types same as val 
 */

#define GP_GET_BITS(offset, count, val) ( ( (val)>>(offset) ) & ( ((((typeof(val))1)<<(count)) - 1) ) )

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

#define GP_CLEAR_BITS(offset, count, dest) ( (dest) &= ~(((((typeof(dest))1) << (count)) - 1) << (offset)) )

#define GP_SET_BITS_OR(offset, dest, val) ( (dest) |= ((val)<<(offset)) )

#define GP_SET_BITS(offset, count, dest, val) (GP_CLEAR_BITS(offset, count, dest), \
					       GP_SET_BITS_OR(offset, dest, val) )


/* 
 * Determines the sign of the integer value; it is +1 if value is positive,
 * -1 if negative, and 0 if it is zero.
 */

#define GP_SIGN(a) ({ \
	typeof(a) _a = a; \
	(_a > 0) ? 1 : ((_a < 0) ? -1 : 0); \
})

#endif /* GP_COMMON_H */
