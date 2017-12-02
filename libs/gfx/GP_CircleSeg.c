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
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "core/GP_GetPutPixel.h"
#include "core/GP_FnPerBpp.h"

#include "gfx/GP_CircleSeg.h"
#include "gfx/GP_HLine.h"

#include "algo/CircleSeg.algo.h"

static uint8_t transform_segments(GP_Pixmap *pixmap, uint8_t seg_flags)
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

/* Generate drawing functions for various bit depths. */
GP_DEF_DRAW_FN_PER_BPP(GP_CircleSeg_Raw, DEF_CIRCLE_SEG_FN)

void GP_CircleSeg_Raw(GP_Pixmap *pixmap, GP_Coord xcenter, GP_Coord ycenter,
                      GP_Size r, uint8_t seg_flag, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(GP_CircleSeg_Raw, pixmap, pixmap,
	                      xcenter, ycenter, r, seg_flag, pixel);
}

void GP_CircleSeg(GP_Pixmap *pixmap, GP_Coord xcenter, GP_Coord ycenter,
                  GP_Size r, uint8_t seg_flag, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);

	GP_CircleSeg_Raw(pixmap, xcenter, ycenter, r,
	                 transform_segments(pixmap, seg_flag), pixel);
}

void GP_FillCircleSeg(GP_Pixmap *pixmap, GP_Coord xcenter, GP_Coord ycenter,
                      GP_Size r, uint8_t seg_flag, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, xcenter, ycenter);

	GP_FillCircleSeg_Raw(pixmap, xcenter, ycenter, r,
	                     transform_segments(pixmap, seg_flag), pixel);
}
