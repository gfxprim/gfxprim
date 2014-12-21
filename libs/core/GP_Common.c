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
 * Copyright (C) 2011-2012 Tomas Gavenciak <gavento@ucw.cz>                  *
 * Copyright (C) 2012      Cyril Hrubis    <metan@ucw.cz>                    *
 *                                                                           *
 *****************************************************************************/

#define _GNU_SOURCE

#include "../config.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif /* HAVE_BACKTRACE */

#ifdef HAVE_DL
#include <dlfcn.h>
#endif /* HAVE_DL */

#include "core/GP_Common.h"

#define GP_ABORT_INFO_TRACE_LEVELS 20


void GP_DebugPrintCStack(void)
{
#ifdef HAVE_BACKTRACE
#if GP_ABORT_INFO_TRACE_LEVELS > 0
	void * buffer[GP_ABORT_INFO_TRACE_LEVELS + 1];
	int size = backtrace(buffer, GP_ABORT_INFO_TRACE_LEVELS);
	fprintf(stderr, "\nC stack trace (most recent call first):\n");
	fflush(stderr);
	backtrace_symbols_fd(buffer, size, fileno(stderr));
#endif /* GP_ABORT_INFO_TRACE_LEVELS > 0 */
#endif /* HAVE_BACKTRACE */
}

static void print_python_stack(void)
{
#ifdef HAVE_DL
	/* Print python stack trace in case python lib is loaded and
         * a python interpreter is initialized */
	int (*dl_Py_IsInitialized)();
	int (*dl_PyRun_SimpleString)(const char *);
	dl_Py_IsInitialized = dlsym(RTLD_DEFAULT, "Py_IsInitialized");
	dl_PyRun_SimpleString = dlsym(RTLD_DEFAULT, "PyRun_SimpleString");
	if (dl_Py_IsInitialized && dl_PyRun_SimpleString && dl_Py_IsInitialized()) {
		fprintf(stderr, "\nPython stack trace (most recent call last; ignore last line):\n");
		fflush(stderr);
		dl_PyRun_SimpleString("import traceback; traceback.print_stack();");
	}
#endif /* HAVE_DL */
}

void GP_PrintAbortInfo(const char *file, const char *func, unsigned int line,
                       const char *msg, const char *fmt, ...)
{
	va_list va;

	fprintf(stderr, "*** gfxprim: %s:%d: in %s: %s\n",
	        file, line, func, msg);

	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);

	fprintf(stderr, "\n");

	print_python_stack();
	GP_DebugPrintCStack();
}
