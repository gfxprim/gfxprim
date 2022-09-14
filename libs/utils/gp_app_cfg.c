//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <stdlib.h>
#include <utils/gp_app_cfg.h>

__attribute__((weak)) char *gp_app_cfg_path(const char *app_name, const char *cfg_filename)
{
	(void) app_name;
	(void) cfg_filename;

	return NULL;
}

__attribute__((weak)) int gp_app_cfg_mkpath(const char *app_name)
{
	(void) app_name;

	errno = ENOSYS;
	return -1;
}
