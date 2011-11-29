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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
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
 * Produces clamped addtion.
 */
GP_Context *GP_FilterAddition(const GP_Context *src_a,
                              const GP_Context *src_b,
                              GP_Context *dst,
                              GP_ProgressCallback *callback);
/*
 * Multiply filter.
 *
 * Produces clamped multiplication.
 */
GP_Context *GP_FilterMultiply(const GP_Context *src_a,
                              const GP_Context *src_b,
                              GP_Context *dst,
                              GP_ProgressCallback *callback);

/*
 * Difference filter.
 *
 * Produces symetric difference.
 * eg. dst = abs(src_a - src_b)
 */
GP_Context *GP_FilterDifference(const GP_Context *src_a,
                                const GP_Context *src_b,
                                GP_Context *dst,
                                GP_ProgressCallback *callback);

/*
 * Maximum filter.
 */
GP_Context *GP_FilterMax(const GP_Context *src_a,
                         const GP_Context *src_b,
			 GP_Context *dst,
			 GP_ProgressCallback *callback);

/*
 * Minimum filter.
 */
GP_Context *GP_FilterMin(const GP_Context *src_a,
                         const GP_Context *src_b,
			 GP_Context *dst,
			 GP_ProgressCallback *callback);

#endif /* FILTERS_GP_ARITHMETIC_H */
