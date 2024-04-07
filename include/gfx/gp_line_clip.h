// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_line_clip.h
 * @brief Line clipping algorithm.
 */
#ifndef GFX_GP_LINECLIP_H
#define GFX_GP_LINECLIP_H

/**
 * @brief Clips a line.
 
 * Clips a line so that the start and end points are within (0, xmax) and
 * (0, ymax). This is used by the line drawing algorithms to make sure that
 * the we only draw the part of line within pixmap.
 *
 * @param px0 A starting x coordinate to clip.
 * @param py0 A starting y cooridinate to clip.
 * @param px1 An ending x coordinate to clip.
 * @param py1 An ending y coordinate to clip.
 * @param xmax A x coordinate upper limit.
 * @param ymax A y coordinate upper limit.
 *
 * @return Zero if the line is completely outside of the pixmap, non-zero otherwise.
 */
int gp_line_clip(int *px0, int *py0, int *px1, int *py1, int xmax, int ymax);

#endif /* GFX_GP_LINE_CLIP */
