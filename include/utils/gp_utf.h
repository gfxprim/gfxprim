// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022-2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_utf.h
 * @brief Unicode helper macros and functions.
 */

#ifndef UTILS_GP_UTF_H
#define UTILS_GP_UTF_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/** Returns true if unicode byte is ASCII */
#define GP_UTF8_IS_ASCII(ch) (!((ch) & 0x80))
/** Returns true if we have first unicode byte of single byte sequence */
#define GP_UTF8_IS_NBYTE(ch) (((ch) & 0xc0) == 0x80)
/** Returns true if we have first unicode byte of two byte sequence */
#define GP_UTF8_IS_2BYTE(ch) (((ch) & 0xe0) == 0xc0)
/** Returns true if we have first unicode byte of three byte sequence */
#define GP_UTF8_IS_3BYTE(ch) (((ch) & 0xf0) == 0xe0)
/** Returns true if we have first unicode byte of four byte sequence */
#define GP_UTF8_IS_4BYTE(ch) (((ch) & 0xf8) == 0xf0)

#define GP_UTF8_NBYTE_MASK 0x3f

/**
 * @brief Parses next unicode character in UTF-8 string.
 * @param str A pointer to the C string.
 * @return A unicode character or 0 on error or end of the string.
 */
static inline uint32_t gp_utf8_next(const char **str)
{
	uint32_t s0 = *str[0];

	(*str)++;

	if (GP_UTF8_IS_ASCII(s0))
		return s0;

	uint32_t s1 = *str[0];

	if (!GP_UTF8_IS_NBYTE(s1))
		return 0;

	s1 &= GP_UTF8_NBYTE_MASK;

	(*str)++;

	if (GP_UTF8_IS_2BYTE(s0))
		return (s0 & 0x1f)<<6 | s1;

	uint32_t s2 = *str[0];

	if (!GP_UTF8_IS_NBYTE(s2))
		return 0;

	s2 &= GP_UTF8_NBYTE_MASK;

	(*str)++;

	if (GP_UTF8_IS_3BYTE(s0))
		return (s0 & 0x0f)<<12 | s1<<6 | s2;

	uint32_t s3 = *str[0];

	if (!GP_UTF8_IS_NBYTE(s3))
		return 0;

	s3 &= GP_UTF8_NBYTE_MASK;

	(*str)++;

	if (GP_UTF8_IS_4BYTE(s0))
		return (s0 & 0x07)<<18 | s1<<12 | s2<<6 | s3;

	return 0;
}

/**
 * @brief Returns number of bytes next character is occupying in an UTF-8 string.
 *
 * @param str A pointer to a string.
 * @param off An offset into the string, must point to a valid multibyte boundary.
 * @return Number of bytes next character occupies, zero on string end and -1 on failure.
 */
int8_t gp_utf8_next_chsz(const char *str, size_t off);

/**
 * @brief Returns number of bytes previous character is occupying in an UTF-8 string.
 *
 * @param str A pointer to a string.
 * @param off An offset into the string, must point to a valid multibyte boundary.
 * @return Number of bytes previous character occupies, and -1 on failure.
 */
int8_t gp_utf8_prev_chsz(const char *str, size_t off);

/**
 * @brief Returns the size of a well-formed UTF-8 sequence.
 *
 * Unlike gp_utf8_next_chsz(), which only checks the bit pattern of each byte,
 * this checks the sequence against the Unicode definition of well-formed
 * UTF-8. Overlong forms, UTF-16 surrogates and anything past U+10FFFF are
 * ill-formed.
 *
 * @param buf A pointer to the sequence.
 * @param avail Number of bytes available at @p buf.
 * @return Size of the sequence 1 to 4 if valid. Zero if avail is zero and -1 if
 *         the sequence is ill-formed or truncated.
 */
int8_t gp_utf8_valid_chsz(const char *buf, size_t avail);

/**
 * @brief Checks that a buffer is well-formed UTF-8.
 *
 * Each sequence is checked as gp_utf8_valid_chsz() does.
 *
 * @param buf A buffer.
 * @param len The buffer length in bytes.
 * @return True if the whole buffer is well-formed UTF-8.
 */
bool gp_utf8_valid(const char *buf, size_t len);

/**
 * @brief Returns a number of characters in UTF-8 string.
 *
 * Returns number of characters in an UTF-8 string, which may be less or equal
 * to what strlen() reports.
 *
 * @param str An UTF-8 string.
 * @return Number of characters in the string.
 */
size_t gp_utf8_strlen(const char *str);

/**
 * @brief Returns a number of bytes needed to store unicode character into UTF-8.
 *
 * @param unicode A unicode character.
 * @return Number of utf8 bytes required to store a unicode character.
 */
static inline unsigned int gp_utf8_bytes(uint32_t unicode)
{
	if (unicode < 0x0080)
		return 1;

	if (unicode < 0x0800)
		return 2;

	if (unicode < 0x10000)
		return 3;

	return 4;
}

/**
 * @brief Writes an unicode character into a UTF-8 buffer.
 *
 * The buffer _must_ be large enough!
 *
 * @param unicode A unicode character.
 * @param buf A byte buffer.
 * @return A number of bytes written.
 */
static inline int gp_to_utf8(uint32_t unicode, char *buf)
{
	if (unicode < 0x0080) {
		buf[0] = unicode & 0x007f;
		return 1;
	}

	if (unicode < 0x0800) {
		buf[0] = 0xc0 | (0x1f & (unicode>>6));
		buf[1] = 0x80 | (0x3f & unicode);
		return 2;
	}

	if (unicode < 0x10000) {
		buf[0] = 0xe0 | (0x0f & (unicode>>12));
		buf[1] = 0x80 | (0x3f & (unicode>>6));
		buf[2] = 0x80 | (0x3f & unicode);
		return 3;
	}

	buf[0] = 0xf0 | (0x07 & (unicode>>18));
	buf[1] = 0x80 | (0x3f & (unicode>>12));
	buf[2] = 0x80 | (0x3f & (unicode>>6));
	buf[3] = 0x80 | (0x3f & unicode);
	return 4;
}

/**
 * @brief Calculates a size for an utf8 string to hold utf16 characters.
 *
 * @param chars An array of utf16 characters.
 * @param chars_len The lenth of the utf16 array.
 *
 * @return A size of the buffer or zero in a case of a size_t overflow. The
 *         returned size includes space for a terminating null character.
 */
static inline size_t gp_utf16_to_utf8_size(uint16_t *chars, size_t chars_len)
{
	size_t ret = 1;
	size_t i;

	for (i = 0; i < chars_len; i++) {
		size_t bytes = gp_utf8_bytes(chars[i]);

		if (bytes + ret < ret)
			return 0;

		ret += bytes;
	}

	return ret;
}

/**
 * @brief Convers utf16 characters into an utf8 string.
 *
 * @param chars An array of utf16 characters.
 * @param chars_len The lenth of the utf16 array.
 *
 * @param dest A buffer large enough to store the utf16 characters and
 *             terminating null character. This can be calculated by
 *             gp_utf16_to_utf8_size().
 */
static inline void gp_utf16_to_utf8(uint16_t *chars, size_t chars_len,
                                    char *dest)
{
	size_t i;

	for (i = 0; i < chars_len; i++)
		dest += gp_to_utf8(chars[i], dest);

	*dest = 0;
}

/**
 * @brief Calculates a size for an utf8 string to hold utf32 characters.
 *
 * @param chars An array of utf32 characters.
 * @param chars_len The lenth of the utf32 array.
 *
 * @return A size of the buffer or zero in a case of a size_t overflow. The
 *         returned size includes space for a terminating null character.
 */
static inline size_t gp_utf32_to_utf8_size(uint32_t *chars, size_t chars_len)
{
	size_t ret = 1;
	size_t i;

	for (i = 0; i < chars_len; i++) {
		size_t bytes = gp_utf8_bytes(chars[i]);

		if (bytes + ret < ret)
			return 0;

		ret += bytes;
	}

	return ret;
}

/**
 * @brief Convers utf32 characters into an utf8 string.
 *
 * @param chars An array of utf32 characters.
 * @param chars_len The lenth of the utf16 array.
 *
 * @param dest A buffer large enough to store the utf32 characters and
 *             terminating null character. This can be calculated by
 *             gp_utf32_to_utf8_size().
 */
static inline void gp_utf32_to_utf8(uint32_t *chars, size_t chars_len,
                                    char *dest)
{
	size_t i;

	for (i = 0; i < chars_len; i++)
		dest += gp_to_utf8(chars[i], dest);

	*dest = 0;
}

/**
 * @brief Attempts to strip diacritics from an unicode character.
 *
 * Attempts to strip diacritics and replace symbols with a similar meaning which
 * produces text that can be stil readable even with ASCII only font.
 *
 * @param ch An UTF character.
 * @return A replacemement for a character with the same meaning or original
 *         character if mapping was not found.
 */
uint32_t gp_utf_fallback(uint32_t ch);

/**
 * @brief Returns the uppercase of a Unicode codepoint.
 *
 * Implements simple (1:1) uppercase case mapping per UnicodeData.txt.
 * Complex mappings that expand into multiple codepoints (German ß → SS,
 * Greek final sigma, etc.) are NOT handled — the codepoint is returned
 * unchanged.
 *
 * @param ch A Unicode codepoint.
 * @return The uppercase codepoint, or ch if there is no mapping.
 */
uint32_t gp_utf_toupper(uint32_t ch);

/**
 * @brief Returns the lowercase of a Unicode codepoint.
 *
 * Implements simple (1:1) lowercase case mapping per UnicodeData.txt.
 *
 * @param ch A Unicode codepoint.
 * @return The lowercase codepoint, or ch if there is no mapping.
 */
uint32_t gp_utf_tolower(uint32_t ch);

/**
 * @brief Returns the titlecase of a Unicode codepoint.
 *
 * For most codepoints titlecase is identical to uppercase; a handful
 * of digraphs (e.g. U+01C5 Dž, U+01C8 Lj) have a distinct titlecase
 * form. Falls back to gp_utf_toupper() when there is no explicit
 * titlecase mapping.
 *
 * @param ch A Unicode codepoint.
 * @return The titlecase codepoint, or ch if there is no mapping.
 */
uint32_t gp_utf_totitle(uint32_t ch);

/**
 * @brief Returns true if the codepoint is a Unicode letter.
 *
 * Tests membership in general category L (Lu/Ll/Lt/Lm/Lo) per
 * UnicodeData.txt.
 *
 * @param ch A Unicode codepoint.
 * @return True if ch is a letter, false otherwise.
 */
bool gp_utf_is_letter(uint32_t ch);

/**
 * @brief Returns true if the codepoint is a white space.
 *
 * Tests the White_Space property per PropList.txt, which includes the ASCII
 * white spaces, no-break spaces and the line and paragraph separators.
 *
 * @param ch A Unicode codepoint.
 * @return True if ch is a white space, false otherwise.
 */
bool gp_utf_is_space(uint32_t ch);

/**
 * @brief A Unicode punctuation class.
 *
 * The seven general categories starting with a P in UnicodeData.txt.
 */
enum gp_utf_punct {
	/** @brief Not a punctuation. */
	GP_UTF_PUNCT_NONE = 0,
	/** @brief Connector, category Pc, e.g. '_'. */
	GP_UTF_PUNCT_CONNECTOR,
	/** @brief Dash, category Pd, e.g. '-'. */
	GP_UTF_PUNCT_DASH,
	/** @brief Open, category Ps, e.g. '('. */
	GP_UTF_PUNCT_OPEN,
	/** @brief Close, category Pe, e.g. ')'. */
	GP_UTF_PUNCT_CLOSE,
	/** @brief Initial quote, category Pi, e.g. '«'. */
	GP_UTF_PUNCT_INIT_QUOTE,
	/** @brief Final quote, category Pf, e.g. '»'. */
	GP_UTF_PUNCT_FINAL_QUOTE,
	/** @brief Other, category Po, e.g. '!'. */
	GP_UTF_PUNCT_OTHER,
};

/**
 * @brief Returns the punctuation class of a codepoint.
 *
 * @param ch An unicode codepoint.
 * @return The punctuation class of ch, or GP_UTF_PUNCT_NONE if it is not
 *         punctuation.
 */
enum gp_utf_punct gp_utf_punct_type(uint32_t ch);

/**
 * @brief A line break opportunity.
 */
enum gp_utf_brk {
	/** @brief A line must not be broken. */
	GP_UTF_BRK_NONE = 0,
	/** @brief A line can be broken. */
	GP_UTF_BRK_ALLOWED,
	/** @brief A line must be broken, e.g. after a '\n'. */
	GP_UTF_BRK_MANDATORY,
};

/**
 * @brief Returns a line break opportunity between two codepoints.
 *
 * Implements the UAX #14 line breaking rules that can be decided from the
 * two codepoints alone, the rules that need more context are approximated:
 *
 * - LB9 A combining mark as prev is treated as a letter. To keep combining
 *   sequences together, pass the base character as prev, i.e. when
 *   gp_utf_brk_combining() is true for next and there is no break before it,
 *   keep prev for the following codepoint. A zero width joiner is not
 *   reported as combining, but a line is never broken after it.
 * - LB12a, LB19a, LB30 The East Asian Width exceptions are not implemented.
 * - LB14 to LB17 Spaces between the classes are not considered, e.g. there
 *   is no break between '(' and a letter, but there is one after "( ".
 * - LB15a, LB15b, LB15c The context around quotation marks and the codepoint
 *   after a space are not considered.
 * - LB20a, LB21a The context before prev is not considered.
 * - LB25 Numbers are matched pairwise, as in the UAX #14 pair table, e.g.
 *   there is no break in "$(" even when no number follows.
 * - LB28a The context before prev and after next is not considered.
 * - LB30a Regional indicators are never broken apart.
 * - LB30b Only emoji bases are kept with emoji modifiers, the unassigned
 *   pictographic codepoints are not.
 * - Complex context dependent scripts, e.g. Thai, need a dictionary and are
 *   not broken within words.
 *
 * A line can be broken after a run of spaces, i.e. between a space and a
 * non-space, the spaces at the end of the line are usually not rendered.
 *
 * @param prev A codepoint before the break.
 * @param next A codepoint after the break.
 * @return A line break opportunity between prev and next.
 */
enum gp_utf_brk gp_utf_break(uint32_t prev, uint32_t next);

/**
 * @brief Returns true if a codepoint is a combining mark for line breaking.
 *
 * Combining marks, e.g. accents, are kept with the preceding character and
 * break as it does, which is the UAX #14 line break class CM, it includes
 * some control characters as well.
 *
 * @param ch A Unicode codepoint.
 * @return True if ch is a combining mark.
 */
bool gp_utf_brk_combining(uint32_t ch);

#endif /* UTILS_GP_UTF_H */
