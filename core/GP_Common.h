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
		fprintf(stderr, "*** gfxprim: aborted: %s: %s\n", __FUNCTION__, #msg); \
		abort(); \
	} while (0)

/*
 * Perform a runtime check, on failure abort and print a message
 */
#define GP_CHECK(cond) do { \
		if (!(cond)) { \
			fprintf(stderr, "*** gfxprim: runtime check failed: %s: %s\n", \
				__FUNCTION__, #cond); \
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

#endif /* GP_COMMON_H */
