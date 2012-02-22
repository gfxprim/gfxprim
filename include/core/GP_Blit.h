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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 * Copyright (C) 2011,2012 Cyril Hrubis    <metan@ucw.cz>                    *
 *                                                                           *
 *****************************************************************************/

/*
   
   These blits automatically converts pixel types, that's good (and fast), but
   there is a catch. This works rather well when the number of colors per
   pixel/color channel is increased (the gamma correction is still on TODO).
   However when the number of colors is decreased it's generally better to use
   dithering, which will yield into far better (you can use Floyd Steinberg
   filter for that).
   
   Also variants without the _Raw suffix do honor the rotation flags, that may
   get a little tricky as the flags for rotation are put together but don't
   worry althouth there is some algebra involved the result is quite intuitive.

 */

#ifndef CORE_GP_BLIT_H
#define CORE_GP_BLIT_H

/*
 * Blits rectangle from src defined by x0, y0, x1, y1 (x1, y1 included) to dst
 * starting on x2, y2.
 */
void GP_BlitXYXY(const GP_Context *src,
                 GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
                 GP_Context *dst, GP_Coord x2, GP_Coord y2);

/*
 * Blits rectangle from src defined by x0, y0, w0, h0 (uses w0 x h0 pixels) to
 * dst starting on x2, y2.
 */
void GP_BlitXYWH(const GP_Context *src,
                 GP_Coord x0, GP_Coord y0, GP_Size w0, GP_Size h0,
                 GP_Context *dst, GP_Coord x1, GP_Coord y1);

/* The default is XYWH now, will be changed */
static inline void GP_Blit(const GP_Context *src,
                           GP_Coord x0, GP_Coord y0,
                           GP_Size w0, GP_Size h0,
                           GP_Context *dst, GP_Coord x1, GP_Coord y1)
{
	GP_BlitXYWH(src, x0, y0, w0, h0, dst, x1, y1);
}

/*
 * Same as GP_BlitXYXY but doesn't respect rotations. Faster (for now).
 */
void GP_BlitXYXY_Raw(const GP_Context *src,
                     GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1,
		     GP_Context *dst, GP_Coord x2, GP_Coord y2);

/*
 * Same as GP_BlitXYWH but doesn't respect rotations. Faster (for now).
 */
void GP_BlitXYWH_Raw(const GP_Context *src,
                     GP_Coord x0, GP_Coord y0, GP_Size w0, GP_Size h0,
		     GP_Context *dst, GP_Coord x2, GP_Coord y2);

/*
 * Same as GP_Blit but doesn't respect rotations. Faster (for now).
 */
static inline void GP_Blit_Raw(const GP_Context *src,
                               GP_Coord x0, GP_Coord y0,
                               GP_Size w0, GP_Size h0,
                               GP_Context *dst, GP_Coord x1, GP_Coord y1)
{
	GP_BlitXYWH_Raw(src, x0, y0, w0, h0, dst, x1, y1);
}



#endif /* CORE_GP_BLIT_H */
