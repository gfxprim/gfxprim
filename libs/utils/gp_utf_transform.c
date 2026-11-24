// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 *
 * Simple Unicode case folding (toupper/tolower/totitle) and letter /
 * punctuation classification.  Tables are generated from UnicodeData.txt by
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
	if (ch < 0x80)
		return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');

	size_t lo = 0, hi = GP_UTF_LETTER_RANGES_LEN;

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

bool gp_utf_is_space(uint32_t ch)
{
	size_t lo = 0, hi = GP_UTF_SPACE_RANGES_LEN;

	while (lo < hi) {
		size_t mid = (lo + hi)/2;

		if (ch < gp_utf_space_ranges[mid].start)
			hi = mid;
		else if (ch > gp_utf_space_ranges[mid].end)
			lo = mid + 1;
		else
			return true;
	}

	return false;
}

/* The class and the offset are packed into 16 bits */
_Static_assert(GP_UTF_LB_CNT <= 1 << GP_UTF_LB_CLASS_BITS, "Too many line break classes");

static enum gp_utf_lb lb_class(uint32_t ch)
{
	size_t lo, hi;

	if (ch >= GP_UTF_LB_HANGUL_FIRST && ch <= GP_UTF_LB_HANGUL_LAST) {
		if ((ch - GP_UTF_LB_HANGUL_FIRST) % GP_UTF_LB_HANGUL_T_COUNT)
			return GP_UTF_LB_H3;

		return GP_UTF_LB_H2;
	}

	if (ch < GP_UTF_LB_BLOCK_LIMIT) {
		size_t block = ch >> GP_UTF_LB_BLOCK_BITS;
		uint16_t off = ch & ((1 << GP_UTF_LB_BLOCK_BITS) - 1);

		lo = gp_utf_lb_index[block];
		hi = gp_utf_lb_index[block + 1];

		/* The last entry at or before the offset, the first one is at 0 */
		while (hi - lo > 1) {
			size_t mid = (lo + hi)/2;

			if (off < gp_utf_lb_entries[mid] >> GP_UTF_LB_CLASS_BITS)
				hi = mid;
			else
				lo = mid;
		}

		return gp_utf_lb_entries[lo] & ((1 << GP_UTF_LB_CLASS_BITS) - 1);
	}

	/* XX resolved by LB1 */
	if (ch > 0x10ffff)
		return GP_UTF_LB_AL;

	lo = 0;
	hi = GP_UTF_LB_RANGES_LEN;

	/* The last range that starts at or before ch */
	while (hi - lo > 1) {
		size_t mid = (lo + hi)/2;

		if (ch < gp_utf_lb_ranges[mid].start)
			hi = mid;
		else
			lo = mid;
	}

	return gp_utf_lb_ranges[lo].type;
}

#define BIT(cls) (1ull << GP_UTF_LB_##cls)

static inline bool brk_in(enum gp_utf_lb cls, uint64_t mask)
{
	return (1ull << cls) & mask;
}

#define HARD (BIT(BK) | BIT(CR) | BIT(LF))
#define ALPHA (BIT(AL) | BIT(HL))
#define HANGUL (BIT(JL) | BIT(JV) | BIT(JT) | BIT(H2) | BIT(H3))
#define AKSARA (BIT(AK) | BIT(AS))

/*
 * The comments refer to the UAX #14 rules.
 */
enum gp_utf_brk gp_utf_break(uint32_t prev, uint32_t next)
{
	enum gp_utf_lb a = lb_class(prev);
	enum gp_utf_lb b = lb_class(next);

	/* LB4, LB5 Always break after hard line breaks, except CR LF */
	if (a == GP_UTF_LB_CR && b == GP_UTF_LB_LF)
		return GP_UTF_BRK_NONE;

	if (brk_in(a, HARD))
		return GP_UTF_BRK_MANDATORY;

	/* LB6, LB7 Do not break before hard line breaks, spaces and ZW */
	if (brk_in(b, HARD | BIT(SP) | BIT(ZW)))
		return GP_UTF_BRK_NONE;

	/* LB8 Break after zero width space */
	if (a == GP_UTF_LB_ZW)
		return GP_UTF_BRK_ALLOWED;

	/* LB8a Do not break after zero width joiner */
	if (a == GP_UTF_LB_ZWJ)
		return GP_UTF_BRK_NONE;

	/* LB9 Do not break before combining marks, except after spaces */
	if (brk_in(b, BIT(CM) | BIT(ZWJ)) && a != GP_UTF_LB_SP)
		return GP_UTF_BRK_NONE;

	/* LB10 Treat the remaining combining marks as letters */
	if (a == GP_UTF_LB_CM)
		a = GP_UTF_LB_AL;

	if (brk_in(b, BIT(CM) | BIT(ZWJ)))
		b = GP_UTF_LB_AL;

	/* LB11 Do not break before or after word joiner */
	if (a == GP_UTF_LB_WJ || b == GP_UTF_LB_WJ)
		return GP_UTF_BRK_NONE;

	/* LB12 Do not break after glue */
	if (a == GP_UTF_LB_GL)
		return GP_UTF_BRK_NONE;

	/* LB12a Do not break before glue, except after spaces and hyphens */
	if (b == GP_UTF_LB_GL && !brk_in(a, BIT(SP) | BIT(BA) | BIT(HY)))
		return GP_UTF_BRK_NONE;

	/* LB13 Do not break before closing punctuation and '/' */
	if (brk_in(b, BIT(CL) | BIT(CP) | BIT(EX) | BIT(SY)))
		return GP_UTF_BRK_NONE;

	/* LB14 Do not break after opening punctuation */
	if (a == GP_UTF_LB_OP)
		return GP_UTF_BRK_NONE;

	/* LB15d Do not break before infix separators, e.g. ',' */
	if (b == GP_UTF_LB_IS)
		return GP_UTF_BRK_NONE;

	/* LB16 Do not break between closing punctuation and nonstarters */
	if (brk_in(a, BIT(CL) | BIT(CP)) && b == GP_UTF_LB_NS)
		return GP_UTF_BRK_NONE;

	/* LB17 Do not break within B2 runs, e.g. "——" */
	if (a == GP_UTF_LB_B2 && b == GP_UTF_LB_B2)
		return GP_UTF_BRK_NONE;

	/* LB18 Break after spaces */
	if (a == GP_UTF_LB_SP)
		return GP_UTF_BRK_ALLOWED;

	/* LB19 Do not break before or after quotation marks */
	if (a == GP_UTF_LB_QU || b == GP_UTF_LB_QU)
		return GP_UTF_BRK_NONE;

	/* LB20 Break before and after contingent break opportunities */
	if (a == GP_UTF_LB_CB || b == GP_UTF_LB_CB)
		return GP_UTF_BRK_ALLOWED;

	/* LB21 Do not break before hyphens and nonstarters, and after BB */
	if (brk_in(b, BIT(BA) | BIT(HY) | BIT(NS)) || a == GP_UTF_LB_BB)
		return GP_UTF_BRK_NONE;

	/* LB21b Do not break between '/' and a Hebrew letter */
	if (a == GP_UTF_LB_SY && b == GP_UTF_LB_HL)
		return GP_UTF_BRK_NONE;

	/* LB22 Do not break before ellipses, IN is merged into NS by LB21 */

	/* LB23 Do not break between letters and digits */
	if ((brk_in(a, ALPHA) && b == GP_UTF_LB_NU) ||
	    (a == GP_UTF_LB_NU && brk_in(b, ALPHA)))
		return GP_UTF_BRK_NONE;

	/* LB23a Do not break between numeric prefixes and ideographs, and ideographs and postfixes */
	if ((a == GP_UTF_LB_PR && brk_in(b, BIT(ID) | BIT(EB) | BIT(EM))) ||
	    (brk_in(a, BIT(ID) | BIT(EB) | BIT(EM)) && b == GP_UTF_LB_PO))
		return GP_UTF_BRK_NONE;

	/* LB24 Do not break between numeric prefixes and postfixes and letters */
	if ((brk_in(a, BIT(PR) | BIT(PO)) && brk_in(b, ALPHA)) ||
	    (brk_in(a, ALPHA) && brk_in(b, BIT(PR) | BIT(PO))))
		return GP_UTF_BRK_NONE;

	/* LB25 Do not break within numbers, e.g. "-5", "$(12.5)" or "50%" */
	if (brk_in(a, BIT(CL) | BIT(CP) | BIT(NU)) && brk_in(b, BIT(PO) | BIT(PR)))
		return GP_UTF_BRK_NONE;

	if (brk_in(a, BIT(PO) | BIT(PR)) && brk_in(b, BIT(OP) | BIT(NU)))
		return GP_UTF_BRK_NONE;

	if (brk_in(a, BIT(HY) | BIT(IS) | BIT(NU) | BIT(SY)) && b == GP_UTF_LB_NU)
		return GP_UTF_BRK_NONE;

	/* LB26 Do not break Korean syllable blocks */
	if (a == GP_UTF_LB_JL && brk_in(b, BIT(JL) | BIT(JV) | BIT(H2) | BIT(H3)))
		return GP_UTF_BRK_NONE;

	if (brk_in(a, BIT(JV) | BIT(H2)) && brk_in(b, BIT(JV) | BIT(JT)))
		return GP_UTF_BRK_NONE;

	if (brk_in(a, BIT(JT) | BIT(H3)) && b == GP_UTF_LB_JT)
		return GP_UTF_BRK_NONE;

	/* LB27 Treat Korean syllable blocks as ideographs before and after numbers */
	if ((brk_in(a, HANGUL) && b == GP_UTF_LB_PO) ||
	    (a == GP_UTF_LB_PR && brk_in(b, HANGUL)))
		return GP_UTF_BRK_NONE;

	/* LB28 Do not break between letters */
	if (brk_in(a, ALPHA) && brk_in(b, ALPHA))
		return GP_UTF_BRK_NONE;

	/*
	 * LB28a Do not break within orthographic syllables of Brahmic scripts,
	 * a dotted circle stands for a missing base
	 */
	bool a_aksara = brk_in(a, AKSARA) || prev == 0x25cc;
	bool b_aksara = brk_in(b, AKSARA) || next == 0x25cc;

	if ((a == GP_UTF_LB_AP && b_aksara) ||
	    (a_aksara && brk_in(b, BIT(VF) | BIT(VI))) ||
	    (a == GP_UTF_LB_VI && (b == GP_UTF_LB_AK || next == 0x25cc)))
		return GP_UTF_BRK_NONE;

	/* LB29 Do not break between infix separators and letters, e.g. "e.g." */
	if (a == GP_UTF_LB_IS && brk_in(b, ALPHA))
		return GP_UTF_BRK_NONE;

	/* LB30 Do not break between letters, digits and parentheses, e.g. "f(x)" */
	if ((brk_in(a, ALPHA | BIT(NU)) && b == GP_UTF_LB_OP) ||
	    (a == GP_UTF_LB_CP && brk_in(b, ALPHA | BIT(NU))))
		return GP_UTF_BRK_NONE;

	/* LB30a Do not break between regional indicators */
	if (a == GP_UTF_LB_RI && b == GP_UTF_LB_RI)
		return GP_UTF_BRK_NONE;

	/* LB30b Do not break between emoji bases and modifiers */
	if (a == GP_UTF_LB_EB && b == GP_UTF_LB_EM)
		return GP_UTF_BRK_NONE;

	/* LB31 Break everywhere else */
	return GP_UTF_BRK_ALLOWED;
}

bool gp_utf_brk_combining(uint32_t ch)
{
	return lb_class(ch) == GP_UTF_LB_CM;
}

enum gp_utf_punct gp_utf_punct_type(uint32_t ch)
{
	size_t lo = 0, hi = GP_UTF_PUNCT_RANGES_LEN;

	while (lo < hi) {
		size_t mid = (lo + hi)/2;

		if (ch < gp_utf_punct_ranges[mid].start)
			hi = mid;
		else if (ch > gp_utf_punct_ranges[mid].end)
			lo = mid + 1;
		else
			return gp_utf_punct_ranges[mid].type;
	}

	return GP_UTF_PUNCT_NONE;
}
