// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include "markup.h"

static int markup_parse_plaintext(struct markup_tcase *tcase)
{
	return markup_parse(GP_MARKUP_PLAINTEXT, tcase);
}

static struct markup_tcase plain_empty_01 = {
	.markup_str = " ",
	.markup_glyphs = {
		{}
	}
};

static struct markup_tcase plain_empty_02 = {
	.markup_str = " \t \t\t",
	.markup_glyphs = {
		{}
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

const struct tst_suite tst_suite = {
	.suite_name = "plaintext markup testsuite",
	.tests = {
		{.name = "plaintext empty 01",
		 .tst_fn = markup_parse_plaintext,
		 .flags = TST_CHECK_MALLOC,
		 .data = &plain_empty_01},

		{.name = "plaintext empty 02",
		 .tst_fn = markup_parse_plaintext,
		 .flags = TST_CHECK_MALLOC,
		 .data = &plain_empty_02},

		{.name = "plaintext 01",
		 .tst_fn = markup_parse_plaintext,
		 .flags = TST_CHECK_MALLOC,
		 .data = &plain_01},

		{.name = "plaintext 02",
		 .tst_fn = markup_parse_plaintext,
		 .flags = TST_CHECK_MALLOC,
		 .data = &plain_02},

		{.name = NULL},
	}
};
