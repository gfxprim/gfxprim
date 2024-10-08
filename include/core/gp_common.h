// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_common.h
 * @brief Common macros.
 */
#ifndef CORE_GP_COMMON_H
#define CORE_GP_COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#include <core/gp_compiler.h>

/**
 * @brief Returns a minimum of the two numbers.
 *
 * @param a A number.
 * @param b A number.
 * @return A minimum of a and b.
 */
#define GP_MIN(a, b) ({ \
	typeof(a) gp_a__ = (a); \
	typeof(b) gp_b__ = (b); \
	gp_a__ < gp_b__ ? gp_a__ : gp_b__; \
})

/**
 * @brief Returns a minimum of the three numbers.
 *
 * @param a A number.
 * @param b A number.
 * @param c A number.
 * @return A minimum of a, b and c.
 */
#define GP_MIN3(a, b, c) ({ \
	typeof(a) gp_a__ = (a); \
	typeof(b) gp_b__ = (b); \
	typeof(c) gp_c__ = (c); \
	gp_a__ < gp_b__ ? (gp_a__ < gp_c__ ? gp_a__ : gp_c__) : (gp_b__ < gp_c__ ? gp_b__ : gp_c__); \
})

/**
 * @brief Returns a maximum of the two numbers.
 *
 * @param a A number.
 * @param b A number.
 * @return A maximum of a and b.
 */
#define GP_MAX(a, b) ({ \
	typeof(a) gp_a__ = (a); \
	typeof(b) gp_b__ = (b); \
	gp_a__ > gp_b__ ? gp_a__ : gp_b__; \
})

/**
 * @brief Returns a maximum of the three numbers.
 *
 * @param a A number.
 * @param b A number.
 * @param c A number.
 * @return A maximum of a, b and c.
 */
#define GP_MAX3(a, b, c) ({ \
	typeof(a) gp_a__ = (a); \
	typeof(b) gp_b__ = (b); \
	typeof(c) gp_c__ = (c); \
	gp_a__ > gp_b__ ? (gp_a__ > gp_c__ ? gp_a__ : gp_c__) : (gp_b__ > gp_c__ ? gp_b__ : gp_c__); \
})

/**
 * @brief A macro to concatenate two strings
 *
 * @param a A string constant.
 * @param b A string constant.
 *
 * @return A concatenated string.
 */
#define GP_CONCAT2(a, b) a##b

#define GP_UNIQUE_ID__(prefix, suffix) GP_CONCAT2(prefix, suffix)

/**
 * @brief Generates an unique C identifier with a given prefix.
 *
 * This is intended to be used to generate an unique indentifier for functions
 * or structures generated by macros.
 *
 * @param prefix A prefix for the unique ID.
 *
 * @return An unique C identifier with a given prefix.
 */
#define GP_UNIQUE_ID(prefix) GP_UNIQUE_ID__(prefix, __COUNTER__)

/**
 * @brief Returns an absolute value.
 *
 * @param a A number.
 * @return An absolute value of a.
 */
#define GP_ABS(a) ({ \
	typeof(a) gp_a__ = a; \
	gp_a__ > 0 ? gp_a__ : - gp_a__; \
})

/**
 * @brief Computes an absolute value of a difference.
 *
 * @param a A number.
 * @param b A number.
 *
 * @return An absolute value of a difference between a and b.
 */
#define GP_ABS_DIFF(a, b) ({ \
	typeof(a) gp_a__ = a; \
	typeof(b) gp_b__ = b; \
	gp_a__ > gp_b__ ? gp_a__ - gp_b__ : gp_b__ - gp_a__; \
})

/**
 * @brief Aligns a value to be even.
 *
 * @param a An integer number.
 * @return A value aligned to be divisible by two.
 */
#define GP_ALIGN2(a) ({ \
	typeof(a) gp_a__ = a; \
	gp_a__ + (gp_a__%2); \
})

/**
 * @brief Swaps a and b.
 *
 * @attention Modifies the variables passed as parameters.
 *
 * @param a A value to be swapped.
 * @param b A value to be swapped.
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

/**
 * @brief Computes number of elements of a statically defined array size.
 *
 * @param array An array.
 * @return A number of array elements.
 */
#define GP_ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))

/**
 * @brief Converts from a pointer to a struct field to a pointer to the struct
 *        itself.
 *
 * The inverse functions is part of the C language and is called offsetof().
 *
 * @param ptr A pointer to a structure member.
 * @param structure A structure C type e.g. struct foo.
 * @param member A structure member name.
 */
#define GP_CONTAINER_OF(ptr, structure, member) \
	((structure *)((char *)(ptr) - offsetof(structure, member)))

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
	if (GP_UNLIKELY(!(check_cond_))) { \
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
