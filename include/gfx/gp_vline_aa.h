// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Anti Aliased Vertical line.

   The coordinates are in XX.8 fixed point format, see core/GP_FixedPoint.h
   for helper macros.

   For RGB pixmaps gamma correction tables are used to generate correct
   intensity for pixels.

 */

#ifndef GFX_GP_VLINE_AA_H
#define GFX_GP_VLINE_AA_H

#include "core/gp_types.h"

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
