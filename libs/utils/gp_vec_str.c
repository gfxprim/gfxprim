//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdio.h>
#include <stdarg.h>
#include <utils/gp_vec_str.h>

char *gp_vec_printf(char *self, const char *fmt, ...)
{
	va_list ap;
	size_t len;

	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	if (!self)
		self = gp_vec_new(len + 1, 1);
	else
		self = gp_vec_resize(self, len + 1);

	if (!self)
		return NULL;

	va_start(ap, fmt);
	vsprintf(self, fmt, ap);
	va_end(ap);

	return self;
}
