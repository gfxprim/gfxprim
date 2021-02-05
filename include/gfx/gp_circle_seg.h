// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GFX_GP_CIRCLE_H
#define GFX_GP_CIRCLE_H

#include <core/gp_types.h>

/*
 * Quadrants in cartesian space the center is set in the middle of the circle.
 *
 * First segment is where both x and y are possitive, second is where only y is
 * possitive, third is for both x and y negative and the last one for only y
 * negative.
 *
 * Note that on computer screen (and in in-memory bitmaps) cordinates for y
 * grows in the opposite direction to the standard cartesian plane.
 *
 * So first segment is actually down right, second is down left, third is up
 * left, and fourth is up right.
 */
enum gp_circle_segments {
	GP_CIRCLE_SEG1 = 0x01, /* First Quadrant  */
	GP_CIRCLE_SEG2 = 0x02, /* Second Quadrant */
	GP_CIRCLE_SEG3 = 0x04, /* Third Quadrant  */
	GP_CIRCLE_SEG4 = 0x08, /* Fourth Quadrant */
};

/* Circle Segment */

void gp_circle_seg(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                   gp_size r, uint8_t seg_flag, gp_pixel pixel);

void gp_circle_seg_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                       gp_size r, uint8_t seg_flag, gp_pixel pixel);

/* Filled Circle Segment */

void gp_fill_circle_seg(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                        gp_size r, uint8_t seg_flag, gp_pixel pixel);

void gp_fill_circle_seg_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                            gp_size r, uint8_t seg_flag, gp_pixel pixel);

#endif /* GFX_GP_CIRCLE_H */
