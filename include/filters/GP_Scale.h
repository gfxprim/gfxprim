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

  GP_Context resize.

 */

#ifndef GP_SCALE_H
#define GP_SCALE_H

#include "core/GP_Context.h"

/*
 * Nearest neighbour
 * 
 * Faster than others, but produces pixelated images. Works however well for
 * images with sharp edges mostly consisting of big once color parts (eg
 * doesn't blurr the result on upscaling).
 */
GP_Context *GP_Scale_NN(GP_Context *src, GP_Size w, GP_Size h);

/*
 * Bicubic Scaling
 *
 * Works well for upscaling. Not so good for downscaling big images to small
 * ones (looses too much information).
 */
GP_Context *GP_Scale_BiCubic(GP_Context *src, GP_Size w, GP_Size h);


#endif /* GP_SCALE_H */
