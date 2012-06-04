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

  Dithering algorithms.

 */

#ifndef FILTERS_GP_DITHER_H
#define FILTERS_GP_DITHER_H

#include "GP_Filter.h"

/*
 * Classical Floyd-Steinberg. Produces good results and is a little faster than
 * the Hilbert-Peano dithering.
 *
 * The error is distributed to the neighbor pixels as follows:
 * (X denotes current position)
 *
 *
 *          |       |
 *          |   X   |  7/16
 *          |       |
 *   -----------------------
 *          |       |
 *     3/16 |  5/16 |  1/16
 *          |       |
 *
 */

/*
 * Semi internal raw version, use at your own risk.
 */
int GP_FilterFloydSteinberg_RGB888_Raw(const GP_Context *src,
                                            GP_Context *dst,
                                            GP_ProgressCallback *callback);

/*
 * Converts RGB888 24bit image to any RGB or Grayscale bitmap.
 *
 * The source pixel_type MUST BE GP_PIXEL_RGB888.
 *
 * The destination must be at least as large as source.
 *
 * If operation was aborted from within a callback, non-zero is returned.
 */
int GP_FilterFloydSteinberg_RGB888(const GP_Context *src,
                                   GP_Context *dst,
                                   GP_ProgressCallback *callback);

/*
 * If malloc() has failed, or operation was aborted by a callback, NULL is
 * returned.
 */
GP_Context *GP_FilterFloydSteinberg_RGB888_Alloc(const GP_Context *src,
                                                 GP_PixelType pixel_type,
						 GP_ProgressCallback *callback);

/*
 * Hilbert-Peano space filling curve based dithering.
 *
 * The error value is distributed around the Hilbert curve.
 *
 * This dithering introduces a little more noisy result but doesn't create
 * repeating patterns like Floyd-Steinberg which looks generally better to
 * human eye. On the other hand edges tend to be less sharp.
 */

/*
 * Semi internal raw version, use at your own risk.
 */
int GP_FilterHilbertPeano_RGB888_Raw(const GP_Context *src,
                                     GP_Context *dst,
                                     GP_ProgressCallback *callback);

/*
 * Converts RGB888 24bit image to any RGB or Grayscale bitmap.
 *
 * The source pixel_type MUST BE GP_PIXEL_RGB888.
 *
 * The destination must be at least as large as source.
 *
 * If the operation was aborted from within a callback, non-zero is returned.
 */
int GP_FilterHilbertPeano_RGB888(const GP_Context *src,
                                 GP_Context *dst,
                                 GP_ProgressCallback *callback);

/*
 * If malloc() has failed, or operation was aborted by a callback, NULL is
 * returned.
 */
GP_Context *GP_FilterHilbertPeano_RGB888_Alloc(const GP_Context *src,
                                               GP_PixelType pixel_type,
                                               GP_ProgressCallback *callback);

#endif /* FILTERS_GP_DITHER_H */
