// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include "markup.h"

static int markup_parse_gfxprim(struct markup_tcase *tcase)
{
	return markup_parse(GP_MARKUP_GFXPRIM, tcase);
}

static struct markup_tcase plain_empty_01 = {
	.markup_str = " ",
	.markup_glyphs = {
		{},
	}
};

static struct markup_tcase plain_empty_02 = {
	.markup_str = " \t \t\t",
	.markup_glyphs = {
		{},
	}
};

static struct markup_tcase plain_01 = {
	.markup_str = "This is a plain text.",
	.markup_glyphs = {
		{.glyph = 'T'}, {.glyph = 'h'}, {.glyph = 'i'}, {.glyph = 's'},
		{.glyph = ' '},
		{.glyph = 'i'}, {.glyph = 's'},
		{.glyph = ' '},
		{.glyph = 'a'},
		{.glyph = ' '},
		{.glyph = 'p'}, {.glyph = 'l'}, {.glyph = 'a'}, {.glyph = 'i'}, {.glyph = 'n'},
		{.glyph = ' '},
		{.glyph = 't'}, {.glyph = 'e'}, {.glyph = 'x'}, {.glyph = 't'},
		{.glyph = '.'},
		{},
	}
};

static struct markup_tcase plain_02 = {
	.markup_str = " hello\n \nworld ",
	.markup_glyphs = {
		{.glyph = 'h'}, {.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'l'}, {.glyph = 'o'},
		{.glyph = '\n'},
		{.glyph = '\n'},
		{.glyph = 'w'}, {.glyph = 'o'}, {.glyph = 'r'}, {.glyph = 'l'}, {.glyph = 'd'},
		{}
	}
};

static struct markup_tcase bold = {
	.markup_str = "hello*world*",
	.markup_glyphs = {
		{.glyph = 'h'}, {.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'l'}, {.glyph = 'o'},
		{.glyph = 'w', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'o', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'r', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'l', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'd', .fmt = GP_MARKUP_BOLD},
		{}
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "gfxprim markup testsuite",
	.tests = {
		{.name = "gfxprim empty 01",
		 .tst_fn = markup_parse_gfxprim,
		 .flags = TST_CHECK_MALLOC,
		 .data = &plain_empty_01},

		{.name = "gfxprim empty 02",
		 .tst_fn = markup_parse_gfxprim,
		 .flags = TST_CHECK_MALLOC,
		 .data = &plain_empty_02},

		{.name = "gfxprim 01",
		 .tst_fn = markup_parse_gfxprim,
		 .flags = TST_CHECK_MALLOC,
		 .data = &plain_01},

		{.name = "gfxprim 02",
		 .tst_fn = markup_parse_gfxprim,
		 .flags = TST_CHECK_MALLOC,
		 .data = &plain_02},

		{.name = "gfxprim bold",
		 .tst_fn = markup_parse_gfxprim,
		 .flags = TST_CHECK_MALLOC,
		 .data = &bold},

		{.name = NULL},
	}
};
