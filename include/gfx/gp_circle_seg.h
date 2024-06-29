// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_circle_seg.h.
 * @brief A ring and circle drawing algorithms.
 */
#ifndef GFX_GP_CIRCLE_SEG_H
#define GFX_GP_CIRCLE_SEG_H

#include <core/gp_types.h>

/**
 * @brief Quadrants in the cartesian space.
 *
 * The center is set in the middle of the circle. First segment is where both x
 * and y are possitive, second is where only y is possitive, third is for both
 * x and y negative and the last one for only y negative.
 *
 * Note that on computer screen (and in in-memory bitmaps) cordinates for y
 * grows in the opposite direction to the standard cartesian plane.
 *
 * So first segment is actually down right, second is down left, third is up
 * left, and fourth is up right.
 */
enum gp_circle_segments {
	/** @brief First Quadrant. */
	GP_CIRCLE_SEG1 = 0x01,
	/** @brief Second Quadrant. */
	GP_CIRCLE_SEG2 = 0x02,
	/** @brief Third Quadrant. */
	GP_CIRCLE_SEG3 = 0x04,
	/** @brief Fourth Quadrant. */
	GP_CIRCLE_SEG4 = 0x08,
};

/**
 * @brief Draws a circle segment.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to draw into.
 * @param xcenter A circle center coordinate.
 * @param ycenter A circle center coordinate.
 * @param r A circle radius, r=0 draws a single pixel.
 * @param seg_flag A bitmask of circle segments to draw.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_circle_seg(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                   gp_size r, uint8_t seg_flag, gp_pixel pixel);

void gp_circle_seg_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                       gp_size r, uint8_t seg_flag, gp_pixel pixel);

/**
 * @brief Draws a filled circle segment.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to draw into.
 * @param xcenter A circle center coordinate.
 * @param ycenter A circle center coordinate.
 * @param r A circle radius, r=0 draws a single pixel.
 * @param seg_flag A bitmask of circle segments to draw.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_fill_circle_seg(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                        gp_size r, uint8_t seg_flag, gp_pixel pixel);

void gp_fill_circle_seg_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                            gp_size r, uint8_t seg_flag, gp_pixel pixel);

/**
 * @brief Draws a filled ring segment.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to draw into.
 * @param xcenter A circle center coordinate.
 * @param ycenter A circle center coordinate.
 * @param r1 A first circle radius, r=0 draws a single pixel.
 * @param r2 A second circle radius, r=0 draws a single pixel.
 * @param seg_flag A bitmask of circle segments to draw.
 * @param pixel A pixel value to be used for the drawing.
 */
void gp_fill_ring_seg(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                      gp_size r1, gp_size r2, uint8_t seg_flag, gp_pixel pixel);

void gp_fill_ring_seg_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                          gp_size r1, gp_size r2, uint8_t seg_flag, gp_pixel pixel);

#endif /* GFX_GP_CIRCLE_SEG_H */
