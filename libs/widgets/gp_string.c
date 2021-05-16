//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <core/gp_debug.h>
#include <widgets/gp_string.h>

size_t gp_string_arr_size(const char *strings[], unsigned int len)
{
	unsigned int i;
	size_t size = len * sizeof(void*);

	for (i = 0; i < len; i++)
		size += strlen(strings[i]) + 1;

	return size;
}

char **gp_string_arr_copy(const char *strings[], unsigned int len, void *buf)
{
	unsigned int i;
	char **copy = buf;

	buf += len * sizeof(void*);

	for (i = 0; i < len; i++) {
		copy[i] = buf;
		strcpy(buf, strings[i]);
		buf += strlen(strings[i]) + 1;
	}

	return copy;
}

char *gp_aprintf(const char *fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap)+1;
	va_end(ap);

	char *tmp = malloc(len);
	if (!tmp) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	va_start(ap, fmt);
	vsprintf(tmp, fmt, ap);
	va_end(ap);

	return tmp;
}
