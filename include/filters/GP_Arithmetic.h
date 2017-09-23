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

  Arithmetic filters - compute products of two bitmaps.

 */

#ifndef FILTERS_GP_ARITHMETIC_H
#define FILTERS_GP_ARITHMETIC_H

#include "GP_Filter.h"

/*
 * Addition filter.
 *
 * Produces saturated (clamped) addtion.
 */
int GP_FilterAddition(const GP_Pixmap *src_a,
                      const GP_Pixmap *src_b,
                      GP_Pixmap *dst,
                      GP_ProgressCallback *callback);

GP_Pixmap *GP_FilterAdditionAlloc(const GP_Pixmap *src_a,
                                   const GP_Pixmap *src_b,
                                   GP_ProgressCallback *callback);

/*
 * Multiply filter.
 *
 * Produces saturated (clamped) multiplication.
 */
int GP_FilterMultiply(const GP_Pixmap *src_a,
                      const GP_Pixmap *src_b,
                      GP_Pixmap *dst,
                      GP_ProgressCallback *callback);

GP_Pixmap *GP_FilterMultiplyAlloc(const GP_Pixmap *src_a,
                                   const GP_Pixmap *src_b,
                                   GP_ProgressCallback *callback);

/*
 * Difference filter.
 *
 * Produces symetric difference.
 * eg. dst = abs(src_a - src_b)
 */
int GP_FilterDifference(const GP_Pixmap *src_a,
                        const GP_Pixmap *src_b,
                        GP_Pixmap *dst,
                        GP_ProgressCallback *callback);

GP_Pixmap *GP_FilterDifferenceAlloc(const GP_Pixmap *src_a,
                                     const GP_Pixmap *src_b,
                                     GP_ProgressCallback *callback);

/*
 * Maximum filter.
 */
int GP_FilterMax(const GP_Pixmap *src_a,
                 const GP_Pixmap *src_b,
                 GP_Pixmap *dst,
                 GP_ProgressCallback *callback);

GP_Pixmap *GP_FilterMaxAlloc(const GP_Pixmap *src_a,
                              const GP_Pixmap *src_b,
                              GP_ProgressCallback *callback);

/*
 * Minimum filter.
 */
int GP_FilterMin(const GP_Pixmap *src_a,
                 const GP_Pixmap *src_b,
                 GP_Pixmap *dst,
                 GP_ProgressCallback *callback);

GP_Pixmap *GP_FilterMinAlloc(const GP_Pixmap *src_a,
                              const GP_Pixmap *src_b,
                              GP_ProgressCallback *callback);

#endif /* FILTERS_GP_ARITHMETIC_H */
