// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef TEXT_GP_UTF_H
#define TEXT_GP_UTF_H

#include <stdint.h>
#include <stddef.h>

#define GP_UTF8_IS_ASCII(ch) (!((ch) & 0x80))
#define GP_UTF8_IS_NBYTE(ch) (((ch) & 0xc0) == 0x80)
#define GP_UTF8_IS_2BYTE(ch) (((ch) & 0xe0) == 0xc0)
#define GP_UTF8_IS_3BYTE(ch) (((ch) & 0xf0) == 0xe0)
#define GP_UTF8_IS_4BYTE(ch) (((ch) & 0xf8) == 0xf0)

#define GP_UTF8_NBYTE_MASK 0x3f

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

	(*str)++;

	uint32_t s3 = *str[0];

	if (!GP_UTF8_IS_NBYTE(s2))
		return 0;

	s3 &= GP_UTF8_NBYTE_MASK;

	if (GP_UTF8_IS_4BYTE(s0))
		return (s0 & 0x07)<<18 | s1<<12 | s2<<6 | s3;

	return 0;
}

/**
 * Returns number of bytes next character is occupying in an UTF-8 string.
 *
 * @str A pointer to a string.
 * @off An offset into the string, must point to a valid multibyte boundary.
 * @return Number of bytes next character occupies, zero on string end and -1 on failure.
 */
int8_t gp_utf8_next_chsz(const char *str, size_t off);

/**
 * Returns number of bytes previous character is occupying in an UTF-8 string.
 *
 * @str A pointer to a string.
 * @off An offset into the string, must point to a valid multibyte boundary.
 * @return Number of bytes previous character occupies, and -1 on failure.
 */
int8_t gp_utf8_prev_chsz(const char *str, size_t off);

/**
 * Returns number of characters in an UTF-8 string, which may be less or equal
 * to what strlen() reports.
 *
 * @str An UTF-8 string.
 * @return Number of characters in the string.
 */
size_t gp_utf8_strlen(const char *str);

/**
 * Attempts to strip diacritics and replace symbols with a similar meaning which
 * produces text that can be stil readable even with ascii only font.
 *
 * @ch An UTF character.
 * @return A replacemement for a character with the same meaning or original
 *         character if mapping was not found.
 */
uint32_t gp_utf_fallback(uint32_t ch);

#endif /* TEXT_GP_UTF_H */
