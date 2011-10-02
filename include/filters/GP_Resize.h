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

  GP_Context interpolations.

  Nearest Neighbour
  ~~~~~~~~~~~~~~~~~

  Fast, but produces pixelated images. Works however well for images with sharp
  edges mostly consisting of big one color regions (eg doesn't blur the
  result on upscaling).


  Bicubic
  ~~~~~~~
  
  Works well for upscaling as is. To get decent result on downscaling,
  low-pass filter (for example gaussian blur) must be used on original image
  before scaling is done.

 */

#ifndef GP_RESIZE_H
#define GP_RESIZE_H

#include "GP_Filter.h"

typedef enum GP_InterpolationType {
	GP_INTER_NN,    /* Nearest Neighbour */
	GP_INTER_CUBIC, /* Bicubic           */
} GP_InterpolationType;

void GP_FilterResize_Raw(GP_Context *src, GP_Context *res,
                         GP_ProgressCallback callback,
		         GP_InterpolationType type);

GP_Context *GP_FilterResize(GP_Context *src, GP_ProgressCallback callback,
                            GP_InterpolationType type, GP_Size w, GP_Size h);

#endif /* GP_RESIZE_H */
