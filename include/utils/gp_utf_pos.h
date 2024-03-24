// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_utf_pos.h
 * @brief An iterator for an UTF-8 string.
 *
 * Helper functions to move by whole characters in an UTF-8 string. This is
 * mainly used for maitaining cursor position in an UTF-8 strings.
 */

#ifndef UTILS_GP_UTF_POS_H
#define UTILS_GP_UTF_POS_H

#include <utils/gp_utf.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

/**
 * @brief Position in an UTF-8 string.
 */
typedef struct gp_utf8_pos {
	/** Offset in bytes in the string. */
	size_t bytes;
	/** Offset in characters in the string. */
	size_t chars;
} gp_utf8_pos;

/**
 * @brief Moves the position to the start of the string.
 *
 * @return A position at the start of the string.
 */
static inline gp_utf8_pos gp_utf8_pos_first(void)
{
	return (gp_utf8_pos){0, 0};
}

/**
 * @brief Moves the position to the end of the string.
 *
 * The position bytes will point exactly to the null byte at the end of the
 * string. The chars counter will be less or equal to bytes.
 *
 * @param str An UTF-8 string.
 * @return A position at the end of the string.
 */
static inline gp_utf8_pos gp_utf8_pos_last(const char *str)
{
	gp_utf8_pos ret = {0, 0};

	for (;;) {
		int8_t chsz = gp_utf8_next_chsz(str, ret.bytes);

		if (chsz <= 0)
			return ret;

		ret.bytes += chsz;
		ret.chars++;
	}
}

/**
 * @brief Returns true if the position points to the end of the string.
 *
 * @param str An UTF-8 string.
 * @param pos A position in the string.
 * @return True if the position points to the end of the string.
 */
static inline int gp_utf8_pos_at_end(const char *str, gp_utf8_pos pos)
{
	return !str[pos.bytes];
}

/**
 * @brief Returns true if the position points to the start of the string.
 *
 * @param pos A position in the string.
 * @return True if the position points to the start of the string.
 */
static inline int gp_utf8_pos_at_home(gp_utf8_pos pos)
{
	return !pos.bytes;
}

/**
 * @brief Returns true if two positions are equal.
 *
 * @param a A position in a string.
 * @param b A position in a string.
 * @return True if two positions are equal.
 */
static inline int gp_utf8_pos_eq(gp_utf8_pos a, gp_utf8_pos b)
{
	return a.bytes == b.bytes;
}

/**
 * @brief Returns true if position a is greater than position b.
 *
 * @param a A position in a string.
 * @param b A position in a string.
 * @return True if position a is greater than position b.
 */
static inline int gp_utf8_pos_gt(gp_utf8_pos a, gp_utf8_pos b)
{
	return a.bytes > b.bytes;
}

/**
 * @brief Returns true if position a is greater or equal to position b.
 *
 * @param a A position in a string.
 * @param b A position in a string.
 * @return True if position a is greater or equal to position b.
 */
static inline int gp_utf8_pos_ge(gp_utf8_pos a, gp_utf8_pos b)
{
	return a.bytes >= b.bytes;
}

/**
 * @brief Substracts two positions.
 *
 * @param a A position in a string.
 * @param b A position in a string.
 * @return Position a minus the position b.
 */
static inline gp_utf8_pos gp_utf8_pos_sub(gp_utf8_pos a, gp_utf8_pos b)
{
	gp_utf8_pos res = {
		.bytes = a.bytes - b.bytes,
		.chars = a.chars - b.chars,
	};

	return res;
}

/**
 * @brief Adds two positions.
 *
 * @param a A position in a string.
 * @param b A position in a string.
 * @return Position a plust the position b.
 */
static inline gp_utf8_pos gp_utf8_pos_add(gp_utf8_pos a, gp_utf8_pos b)
{
	gp_utf8_pos res = {
		.bytes = a.bytes + b.bytes,
		.chars = a.chars + b.chars,
	};

	return res;
}

/**
 * @brief Returns smaller of two positions.
 *
 * @param a A position in a string.
 * @param b A position in a string.
 * @return A smaller of two positions.
 */
static inline gp_utf8_pos gp_utf8_pos_min(gp_utf8_pos a, gp_utf8_pos b)
{
	if (a.bytes < b.bytes)
		return a;
	else
		return b;
}

/**
 * @brief Returns bigger of two positions.
 *
 * @param a A position in a string.
 * @param b A position in a string.
 * @return A bigger of two positions.
 */
static inline gp_utf8_pos gp_utf8_pos_max(gp_utf8_pos a, gp_utf8_pos b)
{
	if (a.bytes > b.bytes)
		return a;
	else
		return b;
}

/**
 * @brief Moves a position in a string by dir characters.
 *
 * @param str An UTF-8 string.
 * @param cur_pos Pointer to a current position in the UTF-8 string that is
 *                going to move.
 * @param dir A number of characters to move. Negative number moves towards the
 *            start, positive towards the end.
 * @return The remider of characters that we were unable to move because we
 *         reached start/end of the string.
 */
static inline ssize_t gp_utf8_pos_move(const char *str, gp_utf8_pos *cur_pos, ssize_t dir)
{
	ssize_t dirs = dir;

	if (dir > 0) {
		while (dir > 0) {
			int8_t chsz = gp_utf8_next_chsz(str, cur_pos->bytes);

			if (chsz <= 0)
				return dirs - dir;

			cur_pos->bytes += chsz;
			cur_pos->chars++;

			dir--;
		}

		return dirs;
	}

	if (dir < 0) {
		while (dir < 0) {
			int8_t chsz = gp_utf8_prev_chsz(str, cur_pos->bytes);

			if (chsz <= 0)
				return dir - dirs;

			cur_pos->bytes -= chsz;
			cur_pos->chars--;

			dir++;
		}

		return -dirs;
	}

	return 0;
}

/**
 * @brief Moves a single character towards the string start and returns current
 *        character.
 *
 * @param str An UTF-8 string.
 * @param pos Pointer to a current position in the UTF-8 string that is going
 *            to move.
 * @return Current character the position is pointing to after it moved
 *         or 0 if the position points to the start of the string.
 */
static inline uint32_t gp_utf8_pos_prev(const char *str, gp_utf8_pos *pos) {
	int8_t chsz;

	if (!pos->bytes)
		return 0;

	chsz = gp_utf8_prev_chsz(str, pos->bytes);
	if (chsz <= 0)
		return 0;

	pos->bytes -= chsz;
	pos->chars--;

	str += pos->bytes;

	return gp_utf8_next(&str);
}

/**
 * @brief Moves a single character towards the string end and returns current
 *        character.
 *
 * @param str An UTF-8 string.
 * @param pos Pointer to a current position in the UTF-8 string that is going
 *            to move.
 * @return Current character the position is pointing to after it moved
 *         or 0 if the position points to the end of the string.
 */
static inline uint32_t gp_utf8_pos_next(const char *str, gp_utf8_pos *pos)
{
	int8_t chsz;

	chsz = gp_utf8_next_chsz(str, pos->bytes);
	if (chsz <= 0)
		return 0;

	str += pos->bytes;

	pos->bytes += chsz;
	pos->chars++;

	return gp_utf8_next(&str);
}

#endif /* UTILS_GP_UTF_POS_H */
