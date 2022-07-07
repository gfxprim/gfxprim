// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef UTILS_GP_UTF_POS_H
#define UTILS_GP_UTF_POS_H

#include <utils/gp_utf.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

/*
 * Position in an UTF-8 string.
 */
typedef struct gp_utf8_pos {
	size_t bytes;
	size_t chars;
} gp_utf8_pos;

static inline gp_utf8_pos gp_utf8_pos_first(void)
{
	return (gp_utf8_pos){0, 0};
}

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

static inline int gp_utf8_pos_at_end(const char *str, gp_utf8_pos pos)
{
	return !str[pos.bytes];
}

static inline int gp_utf8_pos_at_home(gp_utf8_pos pos)
{
	return !pos.bytes;
}

static inline int gp_utf8_pos_eq(gp_utf8_pos a, gp_utf8_pos b)
{
	return a.bytes == b.bytes;
}

static inline int gp_utf8_pos_gt(gp_utf8_pos a, gp_utf8_pos b)
{
	return a.bytes > b.bytes;
}

static inline int gp_utf8_pos_ge(gp_utf8_pos a, gp_utf8_pos b)
{
	return a.bytes >= b.bytes;
}

static inline gp_utf8_pos gp_utf8_pos_min(gp_utf8_pos a, gp_utf8_pos b)
{
	if (a.bytes < b.bytes)
		return a;
	else
		return b;
}

static inline gp_utf8_pos gp_utf8_pos_max(gp_utf8_pos a, gp_utf8_pos b)
{
	if (a.bytes > b.bytes)
		return a;
	else
		return b;
}

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

#endif /* UTILS_GP_UTF_POS_H */
