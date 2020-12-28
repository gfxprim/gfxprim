// SPDX-License-Identifier: GPL-2.1-or-later
/*

   Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_SEEK_H__
#define GP_SEEK_H__

#include <sys/types.h>
#include <stddef.h>

enum gp_seek_whence {
	GP_SEEK_SET,
	GP_SEEK_CUR,
	GP_SEEK_END,
};

/**
 * @brief Computes position for a seek like parameters.
 *
 * Overflow is not reported, instead a closest reachable position is returned.
 *
 * @whence A whence for offset.
 * @off An offeset to seek to.
 * @cur_pos Current position.
 * @max_pos Maximal reachable offset.
 *
 * @return A new offset.
 */
static inline size_t gp_seek_off(ssize_t off, enum gp_seek_whence whence, size_t cur_pos, size_t max_pos)
{
	switch (whence) {
	case GP_SEEK_SET:
		if (off < 0)
			return 0;

		if ((size_t)off > max_pos)
			return max_pos;

		return off;
	break;
	case GP_SEEK_CUR:
		if (off < 0) {
			if (cur_pos < (size_t)-off)
				return 0;

			return cur_pos + off;
		} else {
			if (cur_pos + (size_t)off > max_pos)
				return max_pos;

			return cur_pos + off;
		}
	break;
	case GP_SEEK_END:
		if (off > 0)
			return max_pos;

		if ((size_t)-off > max_pos)
			return 0;

		return max_pos + off;
	break;
	}

	return cur_pos;
}

#endif /* GP_SEEK_H__ */
