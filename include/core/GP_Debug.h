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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Debug message layer.

   Many places of the library uses debug messages to report warnings, bugs, or
   generally important events (i.e. context has been allocated, filter function
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
enum GP_DebugType {
	GP_DEBUG_TODO  = -1,
	GP_DEBUG_WARN  = -2,
	GP_DEBUG_BUG   = -3,
	GP_DEBUG_FATAL = -4,
};

#define GP_DEFAULT_DEBUG_LEVEL 0

#define GP_DEBUG(level, ...) \
	GP_DebugPrint(level, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_TODO(...) \
	GP_DebugPrint(-1, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_WARN(...) \
	GP_DebugPrint(-2, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_BUG(...) \
	GP_DebugPrint(-3, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_FATAL(...) \
	GP_DebugPrint(-4, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

void GP_DebugPrint(int level, const char *file, const char *function, int line,
                   const char *fmt, ...) __attribute__ ((format (printf, 5, 6)));

/*
 * Sets debug level.
 */
void GP_SetDebugLevel(unsigned int level);

/*
 * Returns current debug level.
 */
unsigned int GP_GetDebugLevel(void);


/*
 * Custom debug message handler structure.
 */
struct GP_DebugMsg {
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
void GP_SetDebugHandler(void (*handler)(const struct GP_DebugMsg *msg));

#endif /* CORE_GP_DEBUG_H */
