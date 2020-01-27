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
 * Copyright (C) 2009-2012 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef CORE_GP_COMMON_H
#define CORE_GP_COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#ifndef __cplusplus

/*
 * Returns a minimum of the two numbers.
 */
#define GP_MIN(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	_a < _b ? _a : _b; \
})

/*
 * Returns maximum from three numbers.
 */
#define GP_MIN3(a, b, c) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	typeof(c) _c = (c); \
	_a < _b ? (_a < _c ? _a : _c) : (_b < _c ? _b : _c); \
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
 * Returns maximum from three numbers.
 */
#define GP_MAX3(a, b, c) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	typeof(c) _c = (c); \
	_a > _b ? (_a > _c ? _a : _c) : (_b > _c ? _b : _c); \
})

/*
 * Returns absolute value.
 */
#define GP_ABS(a) ({ \
	typeof(a) _a = a; \
	_a > 0 ? _a : -_a; \
})

/*
 * Aligns value to be even
 */
#define GP_ALIGN2(a) ({   \
	typeof(a) _a = a; \
	_a + (_a%2);      \
})

/*
 * Swap a and b using an intermediate variable
 */
#define GP_SWAP(a, b) do { \
	typeof(a) tmp = b; \
	b = a;             \
	a = tmp;           \
} while (0)

/* Determines the sign of the integer value; it is +1 if value is positive,
 * -1 if negative, and 0 if it is zero.
 */
#define GP_SIGN(a) ({ \
	typeof(a) _a = a; \
	(_a > 0) ? 1 : ((_a < 0) ? -1 : 0); \
})

#define GP_ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))

#define GP_CONTAINER_OF(ptr, structure, member) \
	((structure *)((char *)(ptr) - offsetof(structure, member)))

#endif /* __cplusplus */

/*
 * The standard likely() and unlikely() used in Kernel
 */
#ifndef likely
	#ifdef __GNUC__
		#define likely(x)       __builtin_expect(!!(x),1)
		#define unlikely(x)     __builtin_expect(!!(x),0)
	#else
		#define likely(x)	x
		#define unlikely(x)	x
	#endif
#endif

#define GP_UNUSED(x) (x)__attribute__ ((unused))

/*
 * Internal macros with common code for GP_ABORT, GP_ASSERT and GP_CHECK.
 * GP_INTERNAL_ABORT takes a message that may contain % (e.g. assert condition)
 * and prints message and calls abort().
 * GP_GENERAL_CHECK is a check with specified message prefix
 * (for assert and check)
 */
#define GP_INTERNAL_ABORT(...) do { \
	gp_print_abort_info(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
	abort(); \
} while (0)

/*
 * Prints C stacktrace.
 */
void gp_debug_print_cstack(void);

/*
 * Print as much trace info as possible. Currently, the (C) call stack and
 * the Python stack if a Python interpreter is set up. In case more wrappers
 * are written, it should print a trace for the currently active.
 */
void gp_print_abort_info(const char *file, const char *function, unsigned int line,
                         const char *msg, const char *fmt, ...)
		         __attribute__ ((format (printf, 5, 6)));

#define GP_GENERAL_CHECK(check_cond_, check_message_, ...) do { \
	if (unlikely(!(check_cond_))) { \
		if (#__VA_ARGS__ [0]) \
			GP_INTERNAL_ABORT(check_message_ #check_cond_, \
			                  "\n" __VA_ARGS__); \
		else \
			GP_INTERNAL_ABORT(check_message_ #check_cond_, " "); \
	} \
} while (0)

/*
 * Aborts and prints the message along with the location in code
 * to stderr. Used for fatal errors.
 *
 * Use as either GP_ABORT(msg) or GP_ABORT(format, params...) where
 * msg and format must be string constants.
 */
#define GP_ABORT(...) \
	GP_INTERNAL_ABORT("\n", __VA_ARGS__)

/*
 * Checks the condition and aborts immediately if it is not satisfied,
 * printing the condition and location in the source.
 * (Intended for checking for bugs within the library itself.)
 *
 * Use as either GP_ASSERT(cond), GP_ASSERT(cond, msg) or
 * GP_ASSERT(cond, format, params...) where msg and format must be string
 * constants.
 */
#define GP_ASSERT(check_cond_, ...) \
	GP_GENERAL_CHECK(check_cond_, "assertion failed: ", ##__VA_ARGS__)

/*
 * Perform a runtime check, on failure abort and print a message.
 * (Intended for user-caused errors like invalid arguments.)
 *
 * Use as either GP_CHECK(cond), GP_CHECK(cond, msg) or
 * GP_CHECK(cond, format, params...) where msg and format must be string
 * constants.
 */
#define GP_CHECK(check_cond_, ...) \
	GP_GENERAL_CHECK(check_cond_, "check failed: ", ##__VA_ARGS__)

#endif /* CORE_GP_COMMON_H */
