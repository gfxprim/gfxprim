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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <errno.h>
#include <core/GP_Common.h>
#include <loaders/GP_Loaders.h>

#include "tst_test.h"

static GP_Loader dummy_loaders[1000];

static int register_max_loaders(void)
{
	unsigned int cnt = 0, i;

	for (;;) {
		if (GP_LoaderRegister(&dummy_loaders[cnt])) {
			if (errno != ENOSPC) {
				tst_msg("Wrong errno %s (%i), expected ENOSPC",
					tst_strerr(errno), errno);
				return TST_FAILED;
			}
			break;
		}
		cnt++;

		if (cnt >= GP_ARRAY_SIZE(dummy_loaders)) {
			tst_msg("Failed to reach the max after %u", cnt);
			return TST_FAILED;
		}
	}

	tst_msg("Registered %u loaders", cnt);

	/* Let's provoke SEGFAULT by walking the loaders list */
	GP_ListLoaders();

	for (i = 0; i < cnt; i++)
		GP_LoaderUnregister(&dummy_loaders[i]);

	for (i = 0; i < cnt; i++) {
		if (GP_LoaderRegister(&dummy_loaders[i])) {
			tst_msg("Failed to register %u loader (max=%u)",
				i, cnt);
			return TST_FAILED;
		}
	}

	for (i = 0; i < cnt; i++)
		GP_LoaderUnregister(&dummy_loaders[cnt - i - 1]);

	return TST_SUCCESS;
}

static int register_loader_twice(void)
{
	int ret;

	if (GP_LoaderRegister(dummy_loaders)) {
		tst_msg("Failed to register loader %s (%i)",
		        tst_strerr(errno), errno);
		return TST_FAILED;
	}

	if (GP_LoaderRegister(dummy_loaders)) {
		if (errno != EEXIST) {
			tst_msg("Loader failed but errno %s (%i)",
			        tst_strerr(errno), errno);
			ret = TST_FAILED;
			goto exit;
		}

		tst_msg("Second attempt to register loader fails with EEXIST");
		ret = TST_SUCCESS;
		goto exit;
	}

	tst_msg("Second attempt to register loader succeded");
	ret = TST_FAILED;
exit:
	GP_LoaderUnregister(dummy_loaders);
	return ret;
}

static GP_Loader test_loader = {
	.fmt_name = "TEST",
	.extensions = {"test", NULL},
};

static int loader_by_extension(void)
{
	const GP_Loader *loader;
	int err = 0;

	if (GP_LoaderRegister(&test_loader)) {
		tst_msg("Failed to register loader %s (%i)",
		        tst_strerr(errno), errno);
		return TST_FAILED;
	}

	loader = GP_MatchExtension("file.jpg");

	if (loader != &GP_JPG) {
		tst_msg("Failed to get JPEG loader");
		err++;
	} else {
		tst_msg("Succeded to get JPEG loader");
	}

	loader = GP_MatchExtension("file.test");

	if (loader != &test_loader) {
		tst_msg("Failed to get registered TEST loader");
		err++;
	} else {
		tst_msg("Succeded to get TEST loader");
	}

	GP_LoaderUnregister(&test_loader);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Loader",
	.tests = {
		/* Loader Register tests */
		{.name = "LoaderRegister() max",
		 .tst_fn = register_max_loaders,
		 .flags = TST_CHECK_MALLOC},

		{.name = "LoaderRegister() twice",
		 .tst_fn = register_loader_twice,
		 .flags = TST_CHECK_MALLOC},

		{.name = "MatchExtension()",
		 .tst_fn = loader_by_extension,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
