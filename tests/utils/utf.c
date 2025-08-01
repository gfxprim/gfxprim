// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2022-2025 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
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

		{}
	}
};
