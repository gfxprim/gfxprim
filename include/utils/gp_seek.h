// SPDX-License-Identifier: LGPL-2.1-or-later
/*

   Copyright (C) 2020-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_seek.h
 * @brief Seek contants and transformations.
 */

#ifndef GP_SEEK_H
#define GP_SEEK_H

#include <sys/types.h>
#include <stddef.h>

/**
 * @brief Seek constants.
 *
 * Values are 1:1 with constants passed to lseek()
 */
enum gp_seek_whence {
	GP_SEEK_SET = 0,
	GP_SEEK_CUR = 1,
	GP_SEEK_END = 2,
};

/**
 * @brief Computes position for a seek like parameters.
 *
 * @param whence A whence for offset.
 * @param off An offeset to seek to.
 * @param cur_pos Pointer to a current postion, set to new offset upon successful exit.
 * @param max_pos Maximal reachable offset.
 *
 * @return A zero if new offset is between 0 and max_pos, -1 if new offset is
 *         smaller than zero and 1 if it's greater than max_pos.
 */
static inline int gp_seek_off(ssize_t off, enum gp_seek_whence whence, size_t *cur_pos, size_t max_pos)
{
	switch (whence) {
	case GP_SEEK_SET:
		if (off < 0)
			return -1;

		if ((size_t)off > max_pos)
			return 1;

		*cur_pos = off;

		return 0;
	break;
	case GP_SEEK_CUR:
		if (off < 0) {
			if (*cur_pos < (size_t)-off)
				return -1;
		} else {
			if (*cur_pos + (size_t)off > max_pos)
				return 1;
		}

		*cur_pos += off;

		return 0;
	break;
	case GP_SEEK_END:
		if (off > 0)
			return 1;

		if ((size_t)-off > max_pos)
			return -1;

		*cur_pos = max_pos + off;

		return 0;
	break;
	}

	return -1;
}

#endif /* GP_SEEK_H */
