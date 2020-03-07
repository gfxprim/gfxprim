// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Anti Aliased Horizontal line.

   The coordinates are in XX.8 fixed point format, see core/GP_FixedPoint.h
   for helper macros.

   For RGB pixmaps gamma correction tables are used to generate correct
   intensity for pixels.

 */

#ifndef GFX_GP_HLINE_AA_H
#define GFX_GP_HLINE_AA_H

#include "core/gp_types.h"

/*
 * Anti Aliased Horizontal Line respecting pixmap rotation flags and with
 * clipping.
 */
void gp_hline_aa(gp_pixmap *pixmap, gp_coord x0, gp_coord x1, gp_coord y,
                 gp_pixel pixel);

/*
 * Horizontal Line without contect rotation flags.
 */
void gp_hline_aa_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord x1,
                     gp_coord y, gp_pixel pixel);

#endif /* GFX_GP_HLINE_AA_H */
