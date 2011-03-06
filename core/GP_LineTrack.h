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

#ifndef GP_LINE_TRACK_H
#define GP_LINE_TRACK_H

#define GP_KEEP_FIRST 1
#define GP_KEEP_XMIN 2
#define GP_KEEP_XMAX 4

struct GP_LineTrack {
	int x0, y0;	/* starting point (inclusive) */
	int x1, y1;	/* ending point (inclusive) */
	int flags;	/* flags */

	int x, y;	/* current X and Y position */
	int err;	/* current error term (difference in Y coord from ideal line) */
};

inline void GP_LineTrackInit(struct GP_LineTrack *track, int x0, int y0,
		int x1, int y1, int flags);

inline int GP_LineTrackNext(struct GP_LineTrack *track);

#endif /* GP_LINE_TRACK_H */
