// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2022-2025 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <core/gp_common.h>
#include <utils/gp_utf.h>

#include "tst_test.h"

static int test_utf8_next_01(void)
{
	uint32_t ch;
	const char *str = "\u00a0a";

	ch = gp_utf8_next(&str);
	if (ch != 0xa0) {
		tst_msg("Non breakable space not parsed");
		return TST_FAILED;
	}

	ch = gp_utf8_next(&str);
	if (ch != 'a') {
		tst_msg("ASCII character not parsed");
		return TST_FAILED;
	}

	ch = gp_utf8_next(&str);
	if (ch != 0) {
		tst_msg("End of string not parsed");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int test_utf8_next_03(void)
{
	uint32_t ch;
	const char *str = "a\x8fo";

	ch = gp_utf8_next(&str);
	if (ch != 'a') {
		tst_msg("ASCII character not parsed");
		return TST_FAILED;
	}

	ch = gp_utf8_next(&str);
	if (ch != 0) {
		tst_msg("String not cut on invalid sequence");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int test_utf8_next_04(void)
{
	uint32_t ch;
	const char *str = "\xf0\x9f\x9a\x80 text after rocket emoji";
	const char *str_end = str + 4;

	ch = gp_utf8_next(&str);
	if (ch != 0x1f680) {
		tst_msg("Rocket emoji character not parsed");
		return TST_FAILED;
	}

	if (str != str_end) {
		tst_msg("Wrong offset in string");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int test_utf8_next_02(void)
{
	uint32_t ch;
	const char *str = "\u00a0\xffo";

	ch = gp_utf8_next(&str);
	if (ch != 0xa0) {
		tst_msg("Non breakable space not parsed");
		return TST_FAILED;
	}

	ch = gp_utf8_next(&str);
	if (ch != 0) {
		tst_msg("String not cut on invalid sequence");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int test_utf_fallback(void)
{
	uint32_t ch;

	ch = 0xffff;
	if (gp_utf_fallback(ch) != ch) {
		tst_msg("Got fallback for 0xffff which shouldn't be there");
		return TST_FAILED;
	}

	ch = 0x016f;
	if (gp_utf_fallback(ch) != 'u') {
		tst_msg("Got wrong fallback for 0x016f");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int test_utf8_strlen_01(void)
{
	char *str = "Test \u016f";

	if (gp_utf8_strlen(str) != 6) {
		tst_msg("Got wrong number of characters");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int test_utf8_strlen_02(void)
{
	if (gp_utf8_strlen("\u00a0abc\x8fooo") != 4) {
		tst_msg("Wrong string lenght");
		return TST_FAILED;
	}

	if (gp_utf8_strlen("abc\xffo") != 3) {
		tst_msg("Wrong string lenght");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static struct valid_chsz_case {
	const char *buf;
	size_t avail;
	int8_t ret;
} valid_chsz_cases[] = {
	/* The edges of every row of Unicode 15.0 Table 3-7 */
	{"a", 1, 1},
	{"\0", 1, 1},
	{"\x7f", 1, 1},
	{"\xc2\x80", 2, 2},
	{"\xdf\xbf", 2, 2},
	{"\xe0\xa0\x80", 3, 3},
	{"\xe1\x80\x80", 3, 3},
	{"\xec\xbf\xbf", 3, 3},
	{"\xed\x80\x80", 3, 3},
	{"\xed\x9f\xbf", 3, 3},
	{"\xee\x80\x80", 3, 3},
	{"\xef\xbf\xbf", 3, 3},
	{"\xf0\x90\x80\x80", 4, 4},
	{"\xf1\x80\x80\x80", 4, 4},
	{"\xf3\xbf\xbf\xbf", 4, 4},
	{"\xf4\x80\x80\x80", 4, 4},
	{"\xf4\x8f\xbf\xbf", 4, 4},
	/* Only the first sequence counts */
	{"\xc2\x80zzz", 5, 2},
	/* Nothing to look at */
	{"a", 0, 0},
	/* A stray continuation byte */
	{"\x80", 1, -1},
	{"\xbf", 1, -1},
	/* Overlong forms */
	{"\xc0\xaf", 2, -1},
	{"\xc1\xbf", 2, -1},
	{"\xe0\x80\xaf", 3, -1},
	{"\xe0\x9f\xbf", 3, -1},
	{"\xf0\x80\x80\xaf", 4, -1},
	{"\xf0\x8f\xbf\xbf", 4, -1},
	/* UTF-16 surrogates */
	{"\xed\xa0\x80", 3, -1},
	{"\xed\xbf\xbf", 3, -1},
	/* Past U+10FFFF */
	{"\xf4\x90\x80\x80", 4, -1},
	{"\xf5\x80\x80\x80", 4, -1},
	{"\xff", 1, -1},
	/* A bad continuation byte */
	{"\xc2\x41", 2, -1},
	{"\xe1\x80\x41", 3, -1},
	{"\xf1\x80\x80\xc0", 4, -1},
	/* Truncated by avail, even though the bytes are there */
	{"\xc2\x80", 1, -1},
	{"\xe1\x80\x80", 2, -1},
	{"\xf1\x80\x80\x80", 3, -1},
};

static int test_utf8_valid_chsz(void)
{
	size_t i;
	int fail = 0;

	for (i = 0; i < GP_ARRAY_SIZE(valid_chsz_cases); i++) {
		struct valid_chsz_case *c = &valid_chsz_cases[i];
		int8_t ret = gp_utf8_valid_chsz(c->buf, c->avail);

		if (ret != c->ret) {
			tst_msg("case %zu: got %i expected %i", i, ret, c->ret);
			fail = 1;
		}
	}

	return fail ? TST_FAILED : TST_PASSED;
}

static int test_utf8_valid(void)
{
	static const char text[] = "Test \u016f \xf0\x9f\x9a\x80";
	static const char nul[] = "a\0b";

	if (!gp_utf8_valid("", 0)) {
		tst_msg("Empty buffer is not valid");
		return TST_FAILED;
	}

	if (!gp_utf8_valid(text, sizeof(text) - 1)) {
		tst_msg("Valid buffer rejected");
		return TST_FAILED;
	}

	if (!gp_utf8_valid(nul, sizeof(nul) - 1)) {
		tst_msg("A NUL byte inside the buffer rejected");
		return TST_FAILED;
	}

	if (gp_utf8_valid("ab\xc0\xaf" "c", 5)) {
		tst_msg("Overlong form accepted");
		return TST_FAILED;
	}

	if (gp_utf8_valid("abc\xe2\x82\xac", 5)) {
		tst_msg("Sequence truncated by the length accepted");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int test_utf8_valid_long(void)
{
	char buf[40];
	size_t i;

	memset(buf, 'a', sizeof(buf));

	if (!gp_utf8_valid(buf, sizeof(buf))) {
		tst_msg("Long ASCII buffer rejected");
		return TST_FAILED;
	}

	for (i = 0; i < sizeof(buf); i++) {
		buf[i] = '\xff';
		if (gp_utf8_valid(buf, sizeof(buf))) {
			tst_msg("Bad byte at %zu accepted", i);
			return TST_FAILED;
		}
		buf[i] = 'a';
	}

	for (i = 0; i < sizeof(buf) - 2; i++) {
		memcpy(buf + i, "\xe2\x82\xac", 3);
		if (!gp_utf8_valid(buf, sizeof(buf))) {
			tst_msg("Euro sign at %zu rejected", i);
			return TST_FAILED;
		}
		memset(buf + i, 'a', 3);
	}

	return TST_PASSED;
}

static int test_utf16_to_utf8(void)
{
	char *utf8 = "test\u00a9";
	char utf8_conv[7];
	uint16_t utf16[] = {
		't',
		'e',
		's',
		't',
		0xa9,
	};

	memset(utf8_conv, 0xff, sizeof(utf8_conv));

	size_t conv_len = gp_utf16_to_utf8_size(utf16, 5);

	if (conv_len != 7) {
		tst_msg("Wrong converted size %zu expected 7", conv_len);
		return TST_FAILED;
	}

	gp_utf16_to_utf8(utf16, 5, utf8_conv);

	if (strcmp(utf8, utf8_conv)) {
		tst_msg("Wrong utf8 ('%s') string after conversion!", utf8_conv);
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int test_utf32_to_utf8(void)
{
	char *utf8 = "test\u00a9";
	char utf8_conv[7];
	uint32_t utf32[] = {
		't',
		'e',
		's',
		't',
		0xa9,
	};

	memset(utf8_conv, 0xff, sizeof(utf8_conv));

	size_t conv_len = gp_utf32_to_utf8_size(utf32, 5);

	if (conv_len != 7) {
		tst_msg("Wrong converted size %zu expected 7", conv_len);
		return TST_FAILED;
	}

	gp_utf32_to_utf8(utf32, 5, utf8_conv);

	if (strcmp(utf8, utf8_conv)) {
		tst_msg("Wrong utf8 ('%s') string after conversion!", utf8_conv);
		return TST_FAILED;
	}

	return TST_PASSED;
}

struct case_pair {
	uint32_t in;
	uint32_t expected;
	const char *desc;
};

static int run_case_table(const char *fn_name,
                          uint32_t (*fn)(uint32_t),
                          const struct case_pair *cases, size_t n)
{
	int fails = 0;

	for (size_t i = 0; i < n; i++) {
		uint32_t got = fn(cases[i].in);
		if (got != cases[i].expected) {
			tst_msg("%s(0x%04x) returned 0x%04x, expected 0x%04x [%s]",
			        fn_name, cases[i].in, got, cases[i].expected,
			        cases[i].desc);
			fails++;
		}
	}

	return fails ? TST_FAILED : TST_PASSED;
}

static int test_utf_toupper(void)
{
	static const struct case_pair cases[] = {
		{'a',    'A',    "ASCII a"},
		{'Z',    'Z',    "ASCII Z (already upper)"},
		{'5',    '5',    "digit unchanged"},
		{' ',    ' ',    "space unchanged"},
		{0x00e1, 0x00c1, "Latin a-acute"},
		{0x00fc, 0x00dc, "Latin u-umlaut"},
		{0x010d, 0x010c, "Latin c-caron"},
		{0x017e, 0x017d, "Latin z-caron"},
		{0x00df, 0x00df, "Sharp s (no simple uppercase)"},
		{0x0131, 0x0049, "Dotless i -> I"},
		{0x00d7, 0x00d7, "Multiplication sign (not a letter)"},
		{0xffff, 0xffff, "Unassigned codepoint passes through"},
	};

	return run_case_table("gp_utf_toupper", gp_utf_toupper,
	                      cases, GP_ARRAY_SIZE(cases));
}

static int test_utf_tolower(void)
{
	static const struct case_pair cases[] = {
		{'A',    'a',    "ASCII A"},
		{'z',    'z',    "ASCII z (already lower)"},
		{'5',    '5',    "digit unchanged"},
		{0x00c1, 0x00e1, "Latin A-acute"},
		{0x00dc, 0x00fc, "Latin U-umlaut"},
		{0x010c, 0x010d, "Latin C-caron"},
		{0x017d, 0x017e, "Latin Z-caron"},
		{0x0130, 0x0069, "I-with-dot -> i"},
		{0x00d7, 0x00d7, "Multiplication sign (not a letter)"},
	};

	return run_case_table("gp_utf_tolower", gp_utf_tolower,
	                      cases, GP_ARRAY_SIZE(cases));
}

static int test_utf_totitle(void)
{
	static const struct case_pair cases[] = {
		{'a',    'A',    "ASCII a -> A"},
		{0x00e1, 0x00c1, "a-acute titlecase falls back to uppercase"},
		/* Latin digraphs have distinct titlecase forms (e.g. U+01C4 D\u017d,
		 * U+01C5 D\u017e, U+01C6 d\u017e).  Titlecase of lowercase d\u017e (U+01C6) is
		 * D\u017e (U+01C5), not the all-caps D\u017d (U+01C4). */
		{0x01c4, 0x01c5, "DZ-with-caron titlecase"},
		{0x01c6, 0x01c5, "dz-with-caron titlecase"},
	};

	return run_case_table("gp_utf_totitle", gp_utf_totitle,
	                      cases, GP_ARRAY_SIZE(cases));
}

static int test_utf_is_letter(void)
{
	static const struct {
		uint32_t cp;
		int expected;
		const char *desc;
	} cases[] = {
		{'a',    1, "ASCII a"},
		{'Z',    1, "ASCII Z"},
		{'0',    0, "ASCII digit"},
		{' ',    0, "space"},
		{'!',    0, "ASCII punctuation"},
		{0x00e1, 1, "Latin a-acute"},
		{0x010d, 1, "Latin c-caron"},
		{0x00d7, 0, "Multiplication sign"},
		{0x2603, 0, "Snowman"},
		{0x4e2d, 1, "CJK ideograph"},
		{0x03b1, 1, "Greek alpha"},
		{0x0430, 1, "Cyrillic a"},
	};
	int fails = 0;

	for (size_t i = 0; i < GP_ARRAY_SIZE(cases); i++) {
		int got = gp_utf_is_letter(cases[i].cp);
		if (got != cases[i].expected) {
			tst_msg("gp_utf_is_letter(0x%04x) returned %d, expected %d [%s]",
			        cases[i].cp, got, cases[i].expected, cases[i].desc);
			fails++;
		}
	}

	return fails ? TST_FAILED : TST_PASSED;
}

static int test_utf_punct_type(void)
{
	static const struct {
		uint32_t cp;
		enum gp_utf_punct expected;
		const char *desc;
	} cases[] = {
		{'a',    GP_UTF_PUNCT_NONE,        "ASCII letter"},
		{'0',    GP_UTF_PUNCT_NONE,        "ASCII digit"},
		{' ',    GP_UTF_PUNCT_NONE,        "space"},
		{'+',    GP_UTF_PUNCT_NONE,        "math symbol is not punctuation"},
		{'!',    GP_UTF_PUNCT_OTHER,       "exclamation mark"},
		{'(',    GP_UTF_PUNCT_OPEN,        "left parenthesis"},
		{')',    GP_UTF_PUNCT_CLOSE,       "right parenthesis"},
		{'[',    GP_UTF_PUNCT_OPEN,        "left bracket"},
		{']',    GP_UTF_PUNCT_CLOSE,       "right bracket"},
		{'-',    GP_UTF_PUNCT_DASH,        "hyphen-minus"},
		{'_',    GP_UTF_PUNCT_CONNECTOR,   "low line"},
		{0x00ab, GP_UTF_PUNCT_INIT_QUOTE,  "left guillemet"},
		{0x00bb, GP_UTF_PUNCT_FINAL_QUOTE, "right guillemet"},
		{0x2018, GP_UTF_PUNCT_INIT_QUOTE,  "left single quote"},
		{0x2019, GP_UTF_PUNCT_FINAL_QUOTE, "right single quote"},
		{0x2014, GP_UTF_PUNCT_DASH,        "em dash"},
		{0x2603, GP_UTF_PUNCT_NONE,        "Snowman"},
		{0x10100, GP_UTF_PUNCT_OTHER,      "Aegean word separator"},
	};
	int fails = 0;

	for (size_t i = 0; i < GP_ARRAY_SIZE(cases); i++) {
		enum gp_utf_punct got = gp_utf_punct_type(cases[i].cp);

		if (got != cases[i].expected) {
			tst_msg("gp_utf_punct_type(0x%04x) returned %d, expected %d [%s]",
			        cases[i].cp, got, cases[i].expected, cases[i].desc);
			fails++;
		}
	}

	return fails ? TST_FAILED : TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "utf",
	.tests = {
		{.name = "gp_utf8_next() 01",
		 .tst_fn = test_utf8_next_01},

		{.name = "gp_utf8_next() 02",
		 .tst_fn = test_utf8_next_02},

		{.name = "gp_utf8_next() 03",
		 .tst_fn = test_utf8_next_03},

		{.name = "gp_utf8_next() 04",
		 .tst_fn = test_utf8_next_04},

		{.name = "gp_utf8_strlen() 01",
		 .tst_fn = test_utf8_strlen_01},

		{.name = "gp_utf8_strlen() 02",
		 .tst_fn = test_utf8_strlen_02},

		{.name = "gp_utf8_valid_chsz()",
		 .tst_fn = test_utf8_valid_chsz},

		{.name = "gp_utf8_valid()",
		 .tst_fn = test_utf8_valid},

		{.name = "gp_utf8_valid() long",
		 .tst_fn = test_utf8_valid_long},

		{.name = "gp_utf_fallback()",
		 .tst_fn = test_utf_fallback},

		{.name = "gp_utf16_to_utf8()",
		 .tst_fn = test_utf16_to_utf8},

		{.name = "gp_utf32_to_utf8()",
		 .tst_fn = test_utf32_to_utf8},

		{.name = "gp_utf_toupper()",
		 .tst_fn = test_utf_toupper},

		{.name = "gp_utf_tolower()",
		 .tst_fn = test_utf_tolower},

		{.name = "gp_utf_totitle()",
		 .tst_fn = test_utf_totitle},

		{.name = "gp_utf_is_letter()",
		 .tst_fn = test_utf_is_letter},

		{.name = "gp_utf_punct_type()",
		 .tst_fn = test_utf_punct_type},

		{}
	}
};
