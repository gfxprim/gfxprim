//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <stdlib.h>
#include <utils/gp_user_path.h>

__attribute__((weak))
char *gp_user_path(const char *path, const char *file_name)
{
	(void) path;
	(void) file_name;

	errno = ENOSYS;
	return NULL;
}

__attribute__((weak)) int gp_user_mkpath(const char *path, enum gp_mkpath_flags flags)
{
	(void) path;
	(void) flags;

	errno = ENOSYS;
	return -1;
}
