// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2012 Cyril Hrubis <metan@ucw.cz>
 */

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
