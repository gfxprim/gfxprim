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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Bilinear interpolation.

 */

#ifndef FILTERS_GP_RESIZE_LINEAR_H
#define FILTERS_GP_RESIZE_LINEAR_H

#include "GP_Filter.h"
#include "GP_Resize.h"

int GP_FilterResizeLinearInt(const GP_Context *src, GP_Context *dst,
                             GP_ProgressCallback *callback);

int GP_FilterResizeLinearLFInt(const GP_Context *src, GP_Context *dst,
                             GP_ProgressCallback *callback);

static inline GP_Context *GP_FilterResizeLinearIntAlloc(const GP_Context *src,
                                                        GP_Size w, GP_Size h,
                                                        GP_ProgressCallback *callback)
{
	return GP_FilterResizeAlloc(src, w, h, GP_INTERP_LINEAR_INT, callback);
}

static inline GP_Context *GP_FilterResizeLinearLFIntAlloc(const GP_Context *src,
                                                          GP_Size w, GP_Size h,
                                                          GP_ProgressCallback *callback)
{
	return GP_FilterResizeAlloc(src, w, h, GP_INTERP_LINEAR_LF_INT, callback);
}

#endif /* FILTERS_GP_RESIZE_LINEAR_H */
