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
#include "core/gp_common.h"
#include <loaders/gp_loaders.h>

#include "tst_test.h"

static gp_loader dummy_loaders[1000];

static int register_max_loaders(void)
{
	unsigned int cnt = 0, i;

	for (;;) {
		if (gp_loader_register(&dummy_loaders[cnt])) {
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
	gp_loaders_lists();

	for (i = 0; i < cnt; i++)
		gp_loader_unregister(&dummy_loaders[i]);

	for (i = 0; i < cnt; i++) {
		if (gp_loader_register(&dummy_loaders[i])) {
			tst_msg("Failed to register %u loader (max=%u)",
				i, cnt);
			return TST_FAILED;
		}
	}

	for (i = 0; i < cnt; i++)
		gp_loader_unregister(&dummy_loaders[cnt - i - 1]);

	return TST_SUCCESS;
}

static int register_loader_twice(void)
{
	int ret;

	if (gp_loader_register(dummy_loaders)) {
		tst_msg("Failed to register loader %s (%i)",
		        tst_strerr(errno), errno);
		return TST_FAILED;
	}

	if (gp_loader_register(dummy_loaders)) {
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
	gp_loader_unregister(dummy_loaders);
	return ret;
}

static gp_loader test_loader = {
	.fmt_name = "TEST",
	.extensions = {"test", NULL},
};

static int loader_by_extension(void)
{
	const gp_loader *loader;
	int err = 0;

	if (gp_loader_register(&test_loader)) {
		tst_msg("Failed to register loader %s (%i)",
		        tst_strerr(errno), errno);
		return TST_FAILED;
	}

	loader = gp_loader_by_filename("file.jpg");

	if (loader != &gp_jpg) {
		tst_msg("Failed to get JPEG loader");
		err++;
	} else {
		tst_msg("Succeded to get JPEG loader");
	}

	loader = gp_loader_by_filename("file.test");

	if (loader != &test_loader) {
		tst_msg("Failed to get registered TEST loader");
		err++;
	} else {
		tst_msg("Succeded to get TEST loader");
	}

	gp_loader_unregister(&test_loader);

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

		{.name = "LoaderByFilename()",
		 .tst_fn = loader_by_extension,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
