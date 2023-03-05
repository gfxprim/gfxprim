//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022-2023 Cyril Hrubis <metan@ucw.cz>

 */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>

#include <utils/gp_path.h>
#include <utils/gp_user_path.h>

char *gp_user_path(const char *path, const char *file_name)
{
	char *home_path = gp_user_home();

	if (!home_path)
		return NULL;

	return gp_compose_path(home_path, path, file_name);
}

int gp_user_mkpath(const char *path, enum gp_mkpath_flags flags)
{
	char *home_path = gp_user_home();

	if (!home_path)
		return -1;

	return gp_mkpath(home_path, path, flags, 0700);
}
