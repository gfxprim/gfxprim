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

#endif /* UTILS_GP_UTF_H */
