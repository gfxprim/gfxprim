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

#include <stdarg.h>

#include "GP_Debug.h"

static unsigned int debug_level = GP_DEFAULT_DEBUG_LEVEL;
static int env_used = 0;

void GP_SetDebugLevel(unsigned int level)
{
	debug_level = level;
}

unsigned int GP_GetDebugLevel(void)
{
	return debug_level;
}

void GP_DebugPrint(int level, const char *file, const char *function, int line,
                   const char *fmt, ...)
{
	int i;
	
	if (!env_used) {
		char *level = getenv("GP_DEBUG");
		
		env_used = 1;
		
		if (level != NULL) {
			int new_level = atoi(level);
			
			if (new_level >= 0) {
				debug_level = new_level;

				GP_DEBUG(1, "Using debug level GP_DEBUG=%i "
				             "from enviroment variable",
					     debug_level);
			}
		}
	}
	
	if (level > (int)debug_level)
		return;

	for (i = 1; i < level; i++)
		fputc(' ', stderr);

	switch (level) {
	case -3:
		fprintf(stderr, "*** BUG: %s:%s():%u: ", file, function, line);
	break;
	case -2:
		fprintf(stderr, "*** WARNING: %s:%s():%u: ", file, function, line);
	break;
	case -1:
		fprintf(stderr, "*** TODO: %s:%s():%u: ", file, function, line);
	break;
	default:
		fprintf(stderr, "%u: %s:%s():%u: ",
		        level, file, function, line);
	break;
	}

        va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
        
	fputc('\n', stderr);
}
