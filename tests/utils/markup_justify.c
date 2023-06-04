// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <stdio.h>
#include <utils/gp_markup.h>
#include <utils/gp_markup_justify.h>
#include "tst_test.h"

struct markup_tcase {
	unsigned int width;
	unsigned int exp_lines_cnt;
	gp_markup *markup;
};

static unsigned int width_cb(gp_markup_glyph *first, size_t len, void *priv)
{
	(void) first;
	(void) priv;
	return len;
}

static unsigned int double_width_cb(gp_markup_glyph *first, size_t len, void *priv)
{
	(void) first;
	(void) priv;
	return 2 * len;
}

static int markup_justify(struct markup_tcase *tcase)
{
	gp_markup_lines *lines;

	lines = gp_markup_justify(tcase->markup, tcase->width, width_cb, NULL);
	if (!lines) {
		tst_msg("Failed to justify lines");
		return TST_FAILED;
	}

	printf("---\n");
	gp_markup_justify_dump(lines);
	printf("---\n");

	if (lines->lines_cnt != tcase->exp_lines_cnt) {
		tst_msg("Wrong number of lines %u expected %u\n",
		        lines->lines_cnt, tcase->exp_lines_cnt);
		return TST_FAILED;
	}

	gp_markup_justify_free(lines);

	return TST_PASSED;
}

static struct gp_markup single_newline_markup = {
	.glyph_cnt = 1,
	.glyphs = {
		{.glyph = '\n'},
		{},
	}
};

static struct markup_tcase single_newline = {
	.width = 10,
	.exp_lines_cnt = 1,
	.markup = &single_newline_markup,
};

static struct gp_markup short_text_markup  = {
	.glyph_cnt = 21,
	.glyphs = {
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

static struct gp_markup text_newline_markup  = {
	.glyph_cnt = 21,
	.glyphs = {
		{.glyph = 'T'}, {.glyph = 'h'}, {.glyph = 'i'}, {.glyph = 's'},
		{.glyph = ' '},
		{.glyph = 'i'}, {.glyph = 's'},
		{.glyph = ' '},
		{.glyph = 'a'},
		{.glyph = '\n'},
		{.glyph = 'p'}, {.glyph = 'l'}, {.glyph = 'a'}, {.glyph = 'i'}, {.glyph = 'n'},
		{.glyph = ' '},
		{.glyph = 't'}, {.glyph = 'e'}, {.glyph = 'x'}, {.glyph = 't'},
		{.glyph = '.'},
		{},
	}
};

static struct gp_markup no_spaces_markup  = {
	.glyph_cnt = 19,
	.glyphs = {
		{.glyph = 'T'}, {.glyph = 'h'}, {.glyph = 'i'}, {.glyph = 's'},
		{.glyph = 'i'}, {.glyph = 's'},
		{.glyph = 'a'},
		{.glyph = 'p'}, {.glyph = 'l'}, {.glyph = 'a'}, {.glyph = 'i'}, {.glyph = 'n'},
		{.glyph = 't'}, {.glyph = 'e'}, {.glyph = 'x'}, {.glyph = 't'},
		{.glyph = '.'},
		{.glyph = ' '},
		{.glyph = '*'},
		{},
	}
};

static struct gp_markup long_words_markup  = {
	.glyph_cnt = 38,
	.glyphs = {
		{.glyph = 'L'}, {.glyph = 'o'}, {.glyph = 'r'}, {.glyph = 'e'}, {.glyph = 'm'},
		{.glyph = ' '},
		{.glyph = 'i'}, {.glyph = 'p'}, {.glyph = 's'}, {.glyph = 'u'}, {.glyph = 'm'},
		{.glyph = ' '},
		{.glyph = 'c'}, {.glyph = 'o'}, {.glyph = 'n'}, {.glyph = 's'},
		{.glyph = 'e'}, {.glyph = 'c'}, {.glyph = 't'}, {.glyph = 'e'},
		{.glyph = 't'}, {.glyph = 'u'}, {.glyph = 'r'},
		{.glyph = ' '},
		{.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'i'}, {.glyph = 't'}, {.glyph = ','},
		{.glyph = ' '},
		{.glyph = 'e'}, {.glyph = 'i'}, {.glyph = 'u'}, {.glyph = 's'},
		{.glyph = 'm'}, {.glyph = 'o'}, {.glyph = 'd'}, {.glyph = '.'},
		{},
	}
};

static struct markup_tcase short_text_wide = {
	.width = 11,
	.exp_lines_cnt = 2,
	.markup = &short_text_markup,
};

static struct markup_tcase short_text_narrow = {
	.width = 5,
	.exp_lines_cnt = 4,
	.markup = &short_text_markup,
};

static struct markup_tcase short_text_narrower = {
	.width = 4,
	.exp_lines_cnt = 6,
	.markup = &short_text_markup,
};

static struct markup_tcase text_newline = {
	.width = 8,
	.exp_lines_cnt = 4,
	.markup = &text_newline_markup,
};

static struct markup_tcase no_spaces = {
	.width = 5,
	.exp_lines_cnt = 4,
	.markup = &no_spaces_markup,
};

static struct markup_tcase long_words = {
	.width = 5,
	.exp_lines_cnt = 8,
	.markup = &long_words_markup,
};

static int markup_regression_zero_width(void)
{
	gp_markup_lines *lines;

	lines = gp_markup_justify(&short_text_markup, 0, width_cb, NULL);
	if (lines) {
		printf("---\n");
		gp_markup_justify_dump(lines);
		printf("---\n");
		tst_msg("Failed to fail");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int markup_regression_small_width(void)
{
	gp_markup_lines *lines;

	lines = gp_markup_justify(&short_text_markup, 1, double_width_cb, NULL);
	if (lines) {
		printf("---\n");
		gp_markup_justify_dump(lines);
		printf("---\n");
		tst_msg("Failed to fail");
		return TST_FAILED;
	}

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "markup justify testsuite",
	.tests = {
		{.name = "single newline",
		 .tst_fn = markup_justify,
		 .flags = TST_CHECK_MALLOC,
		 .data = &single_newline},

		{.name = "short text wide",
		 .tst_fn = markup_justify,
		 .flags = TST_CHECK_MALLOC,
		 .data = &short_text_wide},

		{.name = "short text narrow",
		 .tst_fn = markup_justify,
		 .flags = TST_CHECK_MALLOC,
		 .data = &short_text_narrow},

		{.name = "short text narrower",
		 .tst_fn = markup_justify,
		 .flags = TST_CHECK_MALLOC,
		 .data = &short_text_narrower},

		{.name = "no spaces",
		 .tst_fn = markup_justify,
		 .flags = TST_CHECK_MALLOC,
		 .data = &no_spaces},

		{.name = "text newline",
		 .tst_fn = markup_justify,
		 .flags = TST_CHECK_MALLOC,
		 .data = &text_newline},

		{.name = "long words",
		 .tst_fn = markup_justify,
		 .flags = TST_CHECK_MALLOC,
		 .data = &long_words},

		{.name = "regression zero width",
		 .tst_fn = markup_regression_zero_width},

		{.name = "regression small width",
		 .tst_fn = markup_regression_small_width},

		{.name = NULL},
	}
};
