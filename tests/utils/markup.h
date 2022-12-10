// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <ctype.h>
#include <utils/gp_markup_parser.h>
#include "tst_test.h"

#include <stdio.h>

static int markup_cmp(gp_markup_glyph *g1, gp_markup_glyph *g2)
{
	size_t i = 0;

	for (;;) {
		if (g1[i].glyph != g2[i].glyph) {
			tst_msg("%03zu: Glyhp mismatch %04x '%c' != %04x '%c'", i,
				g1[i].glyph,
				isprint(g1[i].glyph) ? g1[i].glyph : ' ',
				g2[i].glyph,
				isprint(g2[i].glyph) ? g2[i].glyph : ' ');
			return TST_FAILED;
		}

		if (!g1[i].glyph)
			return TST_SUCCESS;

		if (g1[i].fmt != g2[i].fmt) {
			tst_msg("%03zu: Text format mismatch %02x != %02x",
			        i, (unsigned int)g1[i].fmt, (unsigned int) g2[i].fmt);
			return TST_FAILED;
		}

		i++;
	}
}

struct markup_tcase {
	const char *markup_str;
	gp_markup_glyph markup_glyphs[];
};

static int markup_parse(enum gp_markup_fmt fmt, struct markup_tcase *tcase)
{
	gp_markup *markup = gp_markup_parse(fmt, tcase->markup_str, 0);

	if (!markup) {
		tst_msg("Failed to parse markup!");
		return TST_FAILED;
	}

	if (markup_cmp(tcase->markup_glyphs, markup->glyphs)) {
		gp_markup_dump(markup);
		return TST_FAILED;
	}

	gp_markup_free(markup);

	return TST_SUCCESS;
}
