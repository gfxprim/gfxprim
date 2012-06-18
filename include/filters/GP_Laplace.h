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

  Laplace filter and Laplace-based filters.

 */

#ifndef FILTERS_GP_LAPLACE_H
#define FILTERS_GP_LAPLACE_H

#include "GP_Filter.h"

/*
 * Laplace, second-derivative filter.
 */
int GP_FilterLaplace(const GP_Context *src, GP_Context *dst,
                     GP_ProgressCallback *callback);

/*
 * Laplace based filter sharpening.
 *
 * The w is direct weight used to multiply the result.
 */
int GP_FilterEdgeSharpening(const GP_Context *src, GP_Context *dst,
                            float w, GP_ProgressCallback *callback);

#endif /* FILTERS_GP_LAPLACE_H */
