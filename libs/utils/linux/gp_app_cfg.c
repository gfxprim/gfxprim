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
#include <utils/gp_app_cfg.h>

char *gp_app_cfg_path(const char *app_name, const char *cfg_filename)
{
	char *home_path = gp_user_home();

	if (!home_path)
		return NULL;

	return gp_compose_path(home_path, ".config", app_name, cfg_filename);
}

int gp_app_cfg_mkpath(const char *app_name)
{
	char *home_path = gp_user_home();
	char *app_path;

	if (!home_path)
		return -1;

	app_path = gp_compose_path(".config", app_name);
	if (!app_path) {
		errno = ENOMEM;
		return -1;
	}

	int ret = gp_mkpath(home_path, app_path, 0, 0700);

	free(app_path);

	return ret;
}

int gp_app_cfg_scanf(const char *app_name, const char *cfg_filename,
                     const char *fmt, ...)
{
	char *path;
	FILE *f;
	va_list va;
	int ret = -1;

	path = gp_app_cfg_path(app_name, cfg_filename);
	if (!path)
		goto ret0;

	f = fopen(path, "r");
	if (!f) {
		if (errno == ENOENT)
			ret = 0;
		goto ret1;
	}

	va_start(va, fmt);
	ret = vfscanf(f, fmt, va);
	va_end(va);

	fclose(f);
ret1:
	free(path);
ret0:
	return ret;
}

int gp_app_cfg_printf(const char *app_name, const char *cfg_filename,
                      const char *fmt, ...)
{
	char *path;
	FILE *f;
	va_list va;
	int ret = -1;

	if (gp_app_cfg_mkpath(app_name))
		goto ret0;

	path = gp_app_cfg_path(app_name, cfg_filename);
	if (!path)
		goto ret0;

	f = fopen(path, "w");
	if (!f)
		goto ret1;

	va_start(va, fmt);
	ret = vfprintf(f, fmt, va);
	va_end(va);

	fclose(f);
ret1:
	free(path);
ret0:
	return ret;
}
