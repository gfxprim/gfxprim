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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 *                                                                           *
 *****************************************************************************/

#define _GNU_SOURCE

#include "../config.h"

#include "core/GP_Common.h"

#ifdef HAVE_DL
#include <dlfcn.h>
#endif /* HAVE_DL */


void SWIG_print_trace(void)
{
#ifdef HAVE_DL
	/* Print python stack trace in case python lib is loaded and
         * a python interpreter is initialized */
	int (*dl_Py_IsInitialized)();
	int (*dl_PyRun_SimpleString)(const char *);
	dl_Py_IsInitialized = dlsym(RTLD_DEFAULT, "Py_IsInitialized");
	dl_PyRun_SimpleString = dlsym(RTLD_DEFAULT, "PyRun_SimpleString");
	if (dl_Py_IsInitialized && dl_PyRun_SimpleString && dl_Py_IsInitialized())
		dl_PyRun_SimpleString("import traceback; traceback.print_stack();");
#endif /* HAVE_DL */
}
