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

#ifndef FILTERS_GP_LINEAR_THREADS_H
#define FILTERS_GP_LINEAR_THREADS_H

#include "GP_Filter.h"

int GP_FilterLinearConvolutionMP_Raw(const GP_Context *src,
                                     GP_Coord x_src, GP_Coord y_src,
                                     GP_Size w_src, GP_Size h_src,
                                     GP_Context *dst,
                                     GP_Coord x_dst, GP_Coord y_dst,
                                     float kernel[], uint32_t kw, uint32_t kh,
                                     float kern_div, GP_ProgressCallback *callback);

int GP_FilterHLinearConvolutionMP_Raw(const GP_Context *src,
                                      GP_Coord x_src, GP_Coord y_src,
                                      GP_Size w_src, GP_Size h_src,
                                      GP_Context *dst,
                                      GP_Coord x_dst, GP_Coord y_dst,
                                      float kernel[], uint32_t kw, float kern_div,
                                      GP_ProgressCallback *callback);

int GP_FilterVLinearConvolutionMP_Raw(const GP_Context *src,
                                      GP_Coord x_src, GP_Coord y_src,
                                      GP_Size w_src, GP_Size h_src,
                                      GP_Context *dst,
                                      GP_Coord x_dst, GP_Coord y_dst,
                                      float kernel[], uint32_t kh, float kern_div,
                                      GP_ProgressCallback *callback);

#endif /* FILTERS_GP_LINEAR_THREADS_H */
