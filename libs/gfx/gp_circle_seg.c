// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_pixmap.h>
#include <core/gp_transform.h>
#include <gfx/gp_circle_seg.h>

static uint8_t transform_segments(gp_pixmap *pixmap, uint8_t seg_flags)
{
	uint8_t seg1 = seg_flags & GP_CIRCLE_SEG1;
	uint8_t seg2 = seg_flags & GP_CIRCLE_SEG2;
	uint8_t seg3 = seg_flags & GP_CIRCLE_SEG3;
	uint8_t seg4 = seg_flags & GP_CIRCLE_SEG4;

	if (pixmap->axes_swap)
		GP_SWAP(seg1, seg3);

	if (pixmap->x_swap) {
		GP_SWAP(seg1, seg2);
		GP_SWAP(seg3, seg4);
	}

	if (pixmap->y_swap) {
		GP_SWAP(seg1, seg4);
		GP_SWAP(seg2, seg3);
	}

	seg1 = seg1 ? GP_CIRCLE_SEG1 : 0;
	seg2 = seg2 ? GP_CIRCLE_SEG2 : 0;
	seg3 = seg3 ? GP_CIRCLE_SEG3 : 0;
	seg4 = seg4 ? GP_CIRCLE_SEG4 : 0;

	return seg1|seg2|seg3|seg4;
}

void gp_circle_seg(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                   gp_size r, uint8_t seg_flag, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);

	gp_circle_seg_raw(pixmap, xcenter, ycenter, r,
	                  transform_segments(pixmap, seg_flag), pixel);
}

void gp_fill_circle_seg(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                        gp_size r, uint8_t seg_flag, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);

	gp_fill_circle_seg_raw(pixmap, xcenter, ycenter, r,
	                       transform_segments(pixmap, seg_flag), pixel);
}
