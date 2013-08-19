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

  GP_Context interpolations.

  Nearest Neighbour
  ~~~~~~~~~~~~~~~~~

  Fast, but produces pixelated images. Works however well for images with sharp
  edges mostly consisting of big one color regions (eg doesn't blur the
  result on upscaling).

  Bilinear
  ~~~~~~~~

  Faster than Bicubic, but less precise.

  Bilinear LF
  ~~~~~~~~~~~

  Bilinear with low-pass filter on downscaling, this is the best choice for
  fast up and downscaling.

  Bicubic
  ~~~~~~~

  Works well for upscaling as is. To get decent result on downscaling,
  low-pass filter (for example gaussian blur) must be used on original image
  before scaling is done.

 */

#ifndef FILTERS_GP_RESIZE_H
#define FILTERS_GP_RESIZE_H

#include "GP_Filter.h"

typedef enum GP_InterpolationType {
	GP_INTERP_NN,            /* Nearest Neighbour                         */
	GP_INTERP_LINEAR_INT,    /* Bilinear - fixed point arithmetics        */
	GP_INTERP_LINEAR_LF_INT, /* Bilinear + low pass filter on downscaling */
	GP_INTERP_CUBIC,         /* Bicubic                                   */
	GP_INTERP_CUBIC_INT,     /* Bicubic - fixed point arithmetics         */
	GP_INTERP_MAX = GP_INTERP_CUBIC_INT,
} GP_InterpolationType;

const char *GP_InterpolationTypeName(enum GP_InterpolationType interp_type);

/*
 * Just interpolate the source context into destination context.
 */
int GP_FilterResize_Raw(const GP_Context *src, GP_Context *dst,
                        GP_InterpolationType type,
                        GP_ProgressCallback *callback);

/*
 * Resize src to fit the dst, both src and dst must have the same pixel_type.
 *
 * Returns non-zero on error (interrupted from callback), zero on success.
 */
int GP_FilterResize(const GP_Context *src, GP_Context *dst,
                    GP_InterpolationType type,
                    GP_ProgressCallback *callback);

/*
 * Resize src to wxh, the result is allocated.
 *
 * Returns pointer to newly created context.
 *
 * Returns NULL in case of failure and errno is set correspondinlgy.
 */
GP_Context *GP_FilterResizeAlloc(const GP_Context *src,
                                 GP_Size w, GP_Size h,
                                 GP_InterpolationType type,
                                 GP_ProgressCallback *callback);

#endif /* FILTERS_GP_RESIZE_H */
