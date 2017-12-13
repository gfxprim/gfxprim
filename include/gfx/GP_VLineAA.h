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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

   Anti Aliased Vertical line.

   The coordinates are in XX.8 fixed point format, see core/GP_FixedPoint.h
   for helper macros.

   For RGB pixmaps gamma correction tables are used to generate correct
   intensity for pixels.

 */

#ifndef GFX_GP_VLINE_AA_H
#define GFX_GP_VLINE_AA_H

#include "core/GP_Pixmap.h"

/*
 * Anti Aliased Horizontal Line respecting pixmap rotation flags and with clipping.
 */
void gp_vline_aa(gp_pixmap *pixmap, gp_coord x, gp_coord y0, gp_coord y1,
                gp_pixel pixel);

/*
 * Horizontal Line without contect rotation flags.
 */
void gp_vline_aa_raw(gp_pixmap *pixmap, gp_coord x,
                     gp_coord y0, gp_coord y1, gp_pixel pixel);

#endif /* GFX_GP_VLINE_AA_H */
