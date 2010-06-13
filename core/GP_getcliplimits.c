/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "GP.h"

inline void GP_GetClipLimits(struct GP_BufferInfo *buffer,
		struct GP_ClipInfo *clip,
		int *min_row, int *max_row, int *min_column, int *max_column)
{
	/* check for infinitely small buffer */
	if (buffer->rows == 0 || buffer->columns == 0) {
		*min_row = 0;
		*max_row = 0;
		*min_column = 0;
		*max_column = 0;
		return;
	}

	/* set row and column limits to the buffer boundaries */
	*min_row = 0;
	*max_row = (int) buffer->rows - 1;
	*min_column = 0;
	*max_column = (int) buffer->columns - 1;

	/* adjust them according to the clip rectangle, if specified */
	if (clip) {

		/* bail out if the clipping rectangle is nonsensical */
		if (clip->min_row > clip->max_row
			|| clip->min_column > clip->max_column) {
			return;
		}

		/* limits can be only narrowed, not widened */
		*min_row = GP_MAX(0, clip->min_row);
		*max_row = GP_MIN(*max_row, clip->max_row);
		*min_column = GP_MAX(0, clip->min_column);
		*max_column = GP_MIN(*max_column, clip->max_column);
	}
}
