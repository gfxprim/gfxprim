// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Debug message layer.

   Many places of the library uses debug messages to report warnings, bugs, or
   generally important events (i.e. pixmap has been allocated, filter function
   has been called).

   Debug messages are printed into the stderr and could be redirected to custom
   handler.

   The verbosity of the messages could be changed by the debug level. The debug
   level is an unsigned integer (by default set to '0') and only messages that have
   debug level lower or equal to debug level are printed.

   There are few special debug message types with negative debug level (that
   means that they are always printed), and as so these are used on various error
   conditions, see below for more information.

  */

#ifndef CORE_GP_DEBUG_H
#define CORE_GP_DEBUG_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Messages with debug level 0 are always printed (you should generally avoid
 * using them unless you wan't user to see the message.)
 *
 * Debug level 1 should be used on object initalization and generally rare and
 * important events.
 *
 * Debug level > 1 is intended for more verbose reporting, like inner cycles
 * or loop debugging.
 *
 * Debug levels with negative level are special.
 *
 * -1   TODO     - not implemented feature
 * -2   WARNING  - generally error that can be recovered
 * -3   BUG      - library gets into unconsistent state
 * -4   FATAL    - fatal condition, not compiled with XYZ support etc.
 */
enum gp_debug_type {
	GP_DEBUG_TODO  = -1,
	GP_DEBUG_WARN  = -2,
	GP_DEBUG_BUG   = -3,
	GP_DEBUG_FATAL = -4,
};

#define GP_DEFAULT_DEBUG_LEVEL 0

#define GP_DEBUG(level, ...) \
	gp_debug_print(level, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_TODO(...) \
	gp_debug_print(-1, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_WARN(...) \
	gp_debug_print(-2, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_BUG(...) \
	gp_debug_print(-3, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_FATAL(...) \
	gp_debug_print(-4, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

void gp_debug_print(int level, const char *file, const char *function, int line,
                   const char *fmt, ...) __attribute__ ((format (printf, 5, 6)));

/*
 * Sets debug level.
 */
void gp_set_debug_level(unsigned int level);

/*
 * Returns current debug level.
 */
unsigned int gp_get_debug_level(void);


/*
 * Custom debug message handler structure.
 */
struct gp_debug_msg {
	int level;
	const char *file;
	const char *fn;
	unsigned int line;
	const char *msg;
};

/*
 * Sets custom debug message handler.
 *
 * If NULL is passed, custom handler is disabled and debug messages are printed
 * into the stderr.
 */
void gp_set_debug_handler(void (*handler)(const struct gp_debug_msg *msg));

#endif /* CORE_GP_DEBUG_H */
