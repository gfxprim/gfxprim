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

  Converts a continuous line of pixels from buffer A to a line of pixels in
  buffer B.

  Supports only trivial conversions i.e. RGB888 to BGR888 and G1_LE to G1_BE,
  etc.

  The code is mainly used in image loaders when saving image from memory buffer
  that has exactly same channels (in size and names) but placed differently in
  buffer of pixel.

 */


#ifndef LOADERS_LINE_CONVERT_H
#define LOADERS_LINE_CONVERT_H

#include "core/GP_Pixel.h"

typedef void (*GP_LineConvert)(const uint8_t *in, uint8_t *out, unsigned int len);

/*
 * The out array is terminated by GP_PIXEL_UNKNOWN.
 *
 * Returns output pixel type given input pixel type and table of posible output
 * types.
 *
 * Returns GP_PIXEL_UNKNOWN if no conversion is posible.
 */
GP_PixelType GP_LineConvertible(GP_PixelType in, GP_PixelType out[]);

/*
 * Returns pointer to conversion function or NULL if there is none.
 */
GP_LineConvert GP_LineConvertGet(GP_PixelType in, GP_PixelType out);

#endif /* LOADERS_LINE_CONVERT_H */
