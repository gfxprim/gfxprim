// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 *
 * Simple Unicode case folding (toupper/tolower/totitle) and letter
 * classification.  Tables are generated from UnicodeData.txt by
 * gen_utf_case.py; only 1:1 simple mappings are covered — context-
 * sensitive and length-changing mappings (e.g. German "sharp s" → "SS",
 * Turkish dotted I) live in SpecialCasing.txt and are not implemented.
 */

#include <stddef.h>
#include <utils/gp_utf.h>
#include "gp_utf_tables.h"

static uint32_t map_lookup(const struct gp_utf_case_entry *tab, size_t len,
                           uint32_t cp)
{
	size_t lo = 0, hi = len;

	while (lo < hi) {
		size_t mid = (lo + hi)/2;
		uint32_t mcp = tab[mid].cp;

		if (mcp == cp)
			return tab[mid].mapped;
		if (mcp < cp)
			lo = mid + 1;
		else
			hi = mid;
	}

	return cp;
}

uint32_t gp_utf_toupper(uint32_t ch)
{
	if (ch < 0x80) {
		if (ch >= 'a' && ch <= 'z')
			return ch - 32;
		return ch;
	}

	return map_lookup(gp_utf_upper_map, GP_UTF_UPPER_MAP_LEN, ch);
}

uint32_t gp_utf_tolower(uint32_t ch)
{
	if (ch < 0x80) {
		if (ch >= 'A' && ch <= 'Z')
			return ch + 32;
		return ch;
	}

	return map_lookup(gp_utf_lower_map, GP_UTF_LOWER_MAP_LEN, ch);
}

uint32_t gp_utf_totitle(uint32_t ch)
{
	uint32_t t;

	if (ch < 0x80)
		return gp_utf_toupper(ch);

	/* Titlecase is identical to uppercase for almost every codepoint;
	 * only digraphs like U+01C5 Dž have a distinct titlecase form. */
	t = map_lookup(gp_utf_title_map, GP_UTF_TITLE_MAP_LEN, ch);
	if (t != ch)
		return t;

	return gp_utf_toupper(ch);
}

bool gp_utf_is_letter(uint32_t ch)
{
	size_t lo, hi;

	if (ch < 0x80)
		return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');

	lo = 0;
	hi = GP_UTF_LETTER_RANGES_LEN;

	while (lo < hi) {
		size_t mid = (lo + hi)/2;

		if (ch < gp_utf_letter_ranges[mid].start)
			hi = mid;
		else if (ch > gp_utf_letter_ranges[mid].end)
			lo = mid + 1;
		else
			return true;
	}

	return false;
}
