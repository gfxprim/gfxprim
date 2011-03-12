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

/* This structure is used to track the starting and ending points of a line
 * drawn by Bresenham algorithm, scanline by scanline (in a top-down manner).
 * Use GP_LineTrackInit() to initialize the structure.
 * Use GP_LineTrackNext() to calculate the next scanline.
 */
struct GP_LineTrack {

	/* these values are constant during the tracking */

	int x0, y0;	/* starting point (inclusive) */
	int x1, y1;	/* ending point (inclusive) */
	int dx;		/* absolute difference between x0 and x1 */
	int dy;		/* ditto for Y */
	int xstep; 	/* +1 if X increases, -1 if it decreases */
	int ystep;	/* ditto for Y */

	/* only the values below are changing during the tracking */

	int x, y;	/* current X and Y position */
	int err;	/* error term (difference in Y from ideal line) */

	/* output values, written by GP_LineTrackNext(), never reused */

	int xmin;	/* minimum X value for the PREVIOUS scanline */
	int xmax;	/* maximum X value for the PREVIOUS scanline */
};

/* Initializes the GP_LineTrack structure and prepares for tracking the line
 * from (x0, y0) to (x1, y1), inclusive.
 * IMPORTANT: At this point, the 'xmin', 'xmax' fields are not yet computed!
 * You must call GP_LineTrackNext() to obtain values for the first scanline.
 */
inline void GP_LineTrackInit(struct GP_LineTrack *track, int x0, int y0, int x1, int y1);

/* Calculates the next scanline of the Bresenham line algorithm,
 * updating the 'x', 'y', 'err' fields and writing 'xmin', 'xmax'.
 */
inline void GP_LineTrackNext(struct GP_LineTrack *track);

#endif /* GP_LINE_TRACK_H */
