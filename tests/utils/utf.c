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

const struct tst_suite tst_suite = {
	.suite_name = "utf",
	.tests = {
		{.name = "gp_utf8_next() 01",
		 .tst_fn = test_utf8_next_01},

		{.name = "gp_utf8_next() 02",
		 .tst_fn = test_utf8_next_02},

		{.name = "gp_utf8_next() 03",
		 .tst_fn = test_utf8_next_03},

		{.name = "gp_utf8_strlen() 01",
		 .tst_fn = test_utf8_strlen_01},

		{.name = "gp_utf8_strlen() 02",
		 .tst_fn = test_utf8_strlen_02},

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

		{}
	}
};
