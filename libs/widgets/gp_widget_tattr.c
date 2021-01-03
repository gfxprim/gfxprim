//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widget_tattr.h>

static size_t attr_len(const char *attrs)
{
	size_t len = 0;

	while (attrs[len] && attrs[len] != '|')
		len++;

	return len;
}

int gp_widget_tattr_parse(const char *attrs, gp_widget_tattr *tattr)
{
	size_t len;
	gp_widget_tattr ret = 0;

	if (!attrs) {
		*tattr = 0;
		return 0;
	}

	while ((len = attr_len(attrs))) {
		if (!strncmp(attrs, "bold", len))
			ret |= GP_TATTR_BOLD;
		else if (!strncmp(attrs, "large", len))
			ret |= GP_TATTR_LARGE;
		else
			return 1;

		attrs += len;

		if (attrs[0] == '|' && attrs[1])
			attrs++;
	}

	if (*attrs)
		return 1;

	*tattr = ret;
	return 0;
}
