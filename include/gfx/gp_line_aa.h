// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Anti Aliased line.

   The coordinates are in XX.8 fixed point format, see core/GP_FixedPoint.h
   for helper macros.

   For RGB pixmaps gamma correction tables are used to generate correct
   intensity for pixels.

 */

#ifndef GFX_GP_LINE_AA_H
#define GFX_GP_LINE_AA_H

#include "core/gp_types.h"

/*
 * Anti Aliased Line respecting pixmap rotation flags and with clipping.
 */
void gp_line_aa(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                gp_coord x1, gp_coord y1, gp_pixel pixel);

/*
 * Line without contect rotation flags.
 */
void gp_line_aa_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                    gp_coord x1, gp_coord y1, gp_pixel pixel);

#endif /* GFX_GP_LINE_AA_H */
