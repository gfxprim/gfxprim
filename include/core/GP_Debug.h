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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*
   
   Debug messages and debug level. Debug level is an unsigned integer.

   Messages with debug level 0 are always printed (you should generally avoid
   using them unless you wan't user to see the message.)

   Debug level 1 should be used on object initalization and generally rare and
   important events.

   Debug level > 1 is intended for more verbose reporting, like inner cycles
   or loop debugging.

   Debug levels with negative level are special. Debug level -1 means TODO,
   level -2 says WARNING while -2 means BUG (i.e. library get into unconsistent
   state).

  */

#ifndef GP_DEBUG_H
#define GP_DEBUG_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define GP_DEFAULT_DEBUG_LEVEL 0

#define GP_DEBUG(level, ...) \
	GP_DebugPrint(level, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_TODO(...) \
	GP_DebugPrint(-1, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_WARN(...) \
	GP_DebugPrint(-2, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define GP_BUG(...) \
	GP_DebugPrint(-3, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

void GP_SetDebugLevel(unsigned int level);

unsigned int GP_GetDebugLevel(void);

void GP_DebugPrint(int level, const char *file, const char *function, int line,
                   const char *fmt, ...) __attribute__ ((format (printf, 5, 6)));

#endif /* GP_DEBUG_H */
