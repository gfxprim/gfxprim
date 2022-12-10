//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <core/gp_common.h>
#include <utils/gp_markup_parser.h>

void gp_markup_dump(gp_markup *self)
{
	size_t i;

	for (i = 0; self->glyphs[i].glyph; i++) {
		switch (self->glyphs[i].glyph) {
		case '\n':
			if (self->glyphs[i].fmt)
				printf("%03zu: -- hline --\n", i);
			else
				printf("%03zu: -- newline --\n", i);
		break;
		default:
			printf("%03zu: '%c' %02x\n", i, self->glyphs[i].glyph, self->glyphs[i].fmt);
		}
	}
}
