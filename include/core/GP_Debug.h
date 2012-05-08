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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
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

  */

#ifndef GP_DEBUG_H
#define GP_DEBUG_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define GP_DEFAULT_DEBUG_LEVEL 0

#define GP_DEBUG(level, ...) do {                                    \
	if (level <= GP_GetDebugLevel()) {                           \
		fprintf(stderr, "%u: %s:%s():%u: ", level, __FILE__, \
		        __FUNCTION__, __LINE__);                     \
		fprintf(stderr, __VA_ARGS__);                        \
		fputc('\n', stderr);                                 \
	}                                                            \
} while (0)

void GP_SetDebugLevel(unsigned int level);

unsigned int GP_GetDebugLevel(void);

#endif /* GP_DEBUG_H */
