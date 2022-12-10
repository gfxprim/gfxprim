//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <core/gp_common.h>
#include <core/gp_debug.h>
#include <utils/gp_markup_parser.h>

gp_markup *gp_markup_parse(enum gp_markup_fmt fmt, const char *markup_str, enum gp_markup_flags flags)
{
	switch (fmt) {
	case GP_MARKUP_PLAINTEXT:
		return gp_markup_plaintext_parse(markup_str, flags);
	case GP_MARKUP_GFXPRIM:
		return gp_markup_gfxprim_parse(markup_str, flags);
	case GP_MARKUP_HTML:
		return gp_markup_html_parse(markup_str, flags);
	}

	GP_BUG("Invalid markup format %i\n", fmt);
	return NULL;
}

void gp_markup_free(gp_markup *self)
{
	free(self);
}
