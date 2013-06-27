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

#include <stdarg.h>
#include <stdio.h>

#include "bogoman_debug.h"

static unsigned int dbg_level = 0;

void bogoman_dbg_print(unsigned int level, const char *file, const char *fn,
                       unsigned int line, const char *fmt, ...)
{
	if (level > dbg_level)
		return;

	if (level == 0)
		fprintf(stderr, "WARNING: %s:%s:%i\n\t", file, fn, line);
	else
		fprintf(stderr, "DEBUG %i:%s:%s:%i:\n\t", level, file, fn, line);

	va_list va;

	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
}

void bogoman_set_dbg_level(unsigned int level)
{
	dbg_level = level;
}
