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

  Point filters, these works on individual pixels.

 */

#ifndef GP_POINT_H
#define GP_POINT_H

#include "core/GP_Context.h"

/*
 * Brightness filter.
 */
void GP_FilterBrightness_Raw(const GP_Context *src, GP_Context *res,
                             int32_t inc);

GP_Context *GP_FilterBrightness(const GP_Context *src, int32_t inc);

/*
 * Contrast filter.
 */
GP_Context *GP_FilterContrast_Raw(const GP_Context *src, GP_Context *res,
                                  float mul);

GP_Context *GP_FilterContrast(const GP_Context *src, float mul);

/*
 * Invert filter.
 */
GP_Context *GP_FilterInvert_Raw(const GP_Context *src, GP_Context *res);

GP_Context *GP_FilterInvert(const GP_Context *src);

#endif /* GP_POINT_H */
