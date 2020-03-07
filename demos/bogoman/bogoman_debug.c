// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

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
