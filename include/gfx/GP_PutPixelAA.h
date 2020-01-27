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
 * Copyright (C) 2012 Cyril Hrubis <metan@ucw.cz>                            *
 *                                                                           *
 *****************************************************************************/

/*

   Puts an anti aliased pixel to pixmap.

   The coordinates are in XX.8 fixed point format, see core/GP_FixedPoint.h
   for helper macros.

   For RGB pixmaps gamma correction tables are used to generate correct
   intensity for pixels.

 */

#ifndef GFX_GP_PUT_PIXEL_AA_H
#define GFX_GP_PUT_PIXEL_AA_H

#include "core/gp_types.h"

/*
 * Anti Aliased Put Pixel respecting pixmap rotation flags and with clipping.
 */
void gp_putpixel_aa(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_pixel pixel);

/*
 * Anti Aliased Put Pixel with clipping.
 */
void gp_putpixel_aa_raw_clipped(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                                gp_pixel pixel);

/*
 * Raw Put Pixel.
 */
void gp_putpixel_aa_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                        gp_pixel pixel);

#endif /* GFX_GP_PUT_PIXEL_AA_H */
