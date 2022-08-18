// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

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
	typeof(a) gp_a__ = (a); \
	typeof(b) gp_b__ = (b); \
	gp_a__ < gp_b__ ? gp_a__ : gp_b__; \
})

/*
 * Returns maximum from three numbers.
 */
#define GP_MIN3(a, b, c) ({ \
	typeof(a) gp_a__ = (a); \
	typeof(b) gp_b__ = (b); \
	typeof(c) gp_c__ = (c); \
	gp_a__ < gp_b__ ? (gp_a__ < gp_c__ ? gp_a__ : gp_c__) : (gp_b__ < gp_c__ ? gp_b__ : gp_c__); \
})

/*
 * Returns a maximum of the two numbers.
 */
#define GP_MAX(a, b) ({ \
	typeof(a) gp_a__ = (a); \
	typeof(b) gp_b__ = (b); \
	gp_a__ > gp_b__ ? gp_a__ : gp_b__; \
})

/*
 * Returns maximum from three numbers.
 */
#define GP_MAX3(a, b, c) ({ \
	typeof(a) gp_a__ = (a); \
	typeof(b) gp_b__ = (b); \
	typeof(c) gp_c__ = (c); \
	gp_a__ > gp_b__ ? (gp_a__ > gp_c__ ? gp_a__ : gp_c__) : (gp_b__ > gp_c__ ? gp_b__ : gp_c__); \
})

/*
 * Returns absolute value.
 */
#define GP_ABS(a) ({ \
	typeof(a) gp_a__ = a; \
	gp_a__ > 0 ? gp_a__ : - gp_a__; \
})

/*
 * Returns absolute value of difference.
 */
#define GP_ABS_DIFF(a, b) ({ \
	typeof(a) gp_a__ = a; \
	typeof(b) gp_b__ = b; \
	gp_a__ > gp_b__ ? gp_a__ - gp_b__ : gp_b__ - gp_a__; \
})

/*
 * Aligns value to be even
 */
#define GP_ALIGN2(a) ({ \
	typeof(a) gp_a__ = a; \
	gp_a__ + (gp_a__%2); \
})

/*
 * Swap a and b using an intermediate variable
 */
#define GP_SWAP(a, b) do { \
	typeof(b) gp_b__ = b; \
	b = a; \
	a = gp_b__; \
} while (0)

/* Determines the sign of the integer value; it is +1 if value is positive,
 * -1 if negative, and 0 if it is zero.
 */
#define GP_SIGN(a) ({ \
	typeof(a) gp_a__ = a; \
	(gp_a__ > 0) ? 1 : ((gp_a__ < 0) ? -1 : 0); \
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
