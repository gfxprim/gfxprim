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

   Linear filters.

 */

#ifndef GP_LINEAR_H
#define GP_LINEAR_H

#include "GP_Filter.h"

/*
 * Gaussian blur
 *
 * The sigma parameters defines the blur radii in horizontal and vertical
 * direction.
 * 
 * This variant could work in-place so it's perectly okay to call
 *
 * GP_FilterGaussianBlur_Raw(context, context, ...);
 */
void GP_FilterGaussianBlur_Raw(GP_Context *src, GP_Context *res,
                               GP_ProgressCallback *callback,
			       float sigma_x, float sigma_y);

GP_Context *GP_FilterGaussianBlur(GP_Context *src,
                                  GP_ProgressCallback *callback,
                                  float sigma_x, float sigma_y);

#endif /* GP_LINEAR_H */
