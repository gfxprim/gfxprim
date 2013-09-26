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

%% extends "base.test.c.t"

%% block descr
Iterate over all pixel types, try to save context but abort it from callback.
%% endblock descr

%% block body

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <core/GP_Context.h>
#include <loaders/GP_Loaders.h>

#include "tst_test.h"

%% import "savers.t" as savers

typedef int (*Save)(const GP_Context *src, const char *path, GP_ProgressCallback *callback);

static int progress_callback(GP_ProgressCallback *self)
{
	(void) self;
	return 1;
}

static int test(Save Saver, GP_PixelType pixel_type)
{
	GP_Context *src;
	GP_ProgressCallback callback ={.callback = progress_callback};
	int ret = TST_SUCCESS;

	src = GP_ContextAlloc(100, 100, pixel_type);

	if (!src) {
		tst_msg("Malloc failed");
		return TST_UNTESTED;
	}

	if (Saver(src, "testfile", &callback)) {
		if (errno == ENOSYS) {
			tst_msg("Unimplemented pixel value");
			ret = TST_SKIPPED;
			goto err;
		}

		if (errno == EINVAL) {
			tst_msg("Invalid pixel value for the format");
			ret = TST_SKIPPED;
			goto err;
		}

		if (errno == ECANCELED) {
			if (access("testfile", F_OK) == 0) {
				tst_msg("Operation canceled but file exists");
				ret = TST_FAILED;
				goto err;
			} else {
				goto err;
			}
		}

		tst_msg("Saver failed with %s", strerror(errno));
		ret = TST_FAILED;
		goto err;
	} else {
		tst_msg("Succedded unexpectedly");
		ret = TST_FAILED;
	}

err:
	GP_ContextFree(src);
	return ret;
}

%% for fmt in savers.fmts
%%  for pt in pixeltypes
%%   if not pt.is_unknown()
static int test_{{ fmt }}_{{ pt.name }}(void)
{
	return test(GP_Save{{ fmt }}, GP_PIXEL_{{ pt.name }});
}

%%   endif
%%  endfor
%% endfor

const struct tst_suite tst_suite = {
	.suite_name = "SaveAbort",
	.tests = {
%% for fmt in savers.fmts
%%  for pt in pixeltypes
%%   if not pt.is_unknown()
		{.name = "{{ fmt }} {{ pt.name }}",
		 .tst_fn = test_{{ fmt }}_{{ pt.name }},
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
%%   endif
%%  endfor
%% endfor
		{.name = NULL},
	}
};

%% endblock body
