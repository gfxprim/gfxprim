//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdio.h>
#include <stdarg.h>
#include <utils/gp_vec_str.h>

char *gp_vec_vprintf(char *self, const char *fmt, va_list va)
{
	size_t len;
	va_list cpy;

	va_copy(cpy, va);
	len = vsnprintf(NULL, 0, fmt, cpy);
	va_end(cpy);

	if (!self)
		self = gp_vec_new(len + 1, 1);
	else
		self = gp_vec_resize(self, len + 1);

	if (!self)
		return NULL;

	vsprintf(self, fmt, va);

	return self;
}

char *gp_vec_printf(char *self, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	self = gp_vec_vprintf(self, fmt, ap);
	va_end(ap);

	return self;
}
