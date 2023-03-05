//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <utils/gp_path.h>

__attribute__((weak))
char *gp_user_home(void)
{
	errno = ENOSYS;
	return NULL;
}

#define PATH_DELIM '/'

static size_t comp_len(const char *str)
{
	size_t ret = strlen(str);

	while (ret > 0 && str[ret-1] == PATH_DELIM)
		ret--;

	return ret;
}

char *gp_compose_path_(const char *dir, ...)
{
	va_list va1, va2;
	const char *cur = dir, *next;
	char *ret;
	size_t len = 1;

	if (!dir)
		return NULL;

	va_start(va1, dir);
	va_copy(va2, va1);
	do {
		size_t clen = comp_len(cur);
		next = va_arg(va1, const char *);

		while (next && *next == PATH_DELIM)
			next++;

		if (next && *next)
			len++;

		len += clen;
		cur = next;
	} while (next);
	va_end(va1);

	ret = malloc(len);
	if (!ret)
		goto exit;

	cur = dir;
	len = 0;
	do {
		size_t clen = comp_len(cur);
		next = va_arg(va2, const char *);

		while (next && *next == PATH_DELIM)
			next++;

		strncpy(ret + len, cur, clen);

		len+=clen;

		if (next && *next)
			ret[len++] = PATH_DELIM;

		cur = next;
	} while (next);

	ret[len] = 0;

exit:
	va_end(va2);
	return ret;
}

__attribute__((weak))
int gp_mkpath(const char *base_path, const char *path,
              enum gp_mkpath_flags flags, int mode)
{
	(void) base_path;
	(void) path;
	(void) flags;
	(void) mode;

	errno = ENOSYS;
	return -1;
}
