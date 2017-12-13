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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*
 * Macros that generate a switch-case block that calls various variants
 * of the specified function depending on the bit depth (bpp).
 *
 * For function name y and x bpp, the function called will be
 * y_xBPP, resp y_xBPP_LE (or _BE; the bit-endian) for x<8.
 *
 * The functions branch either on bpp and bit_endian, type of given
 * gp_pixmap or given gp_pixel_type.
 *
 * Extra arguments are arguments to be passed to the function.
 * Note that if the function takes the pixmap/type/bpp as an argument,
 * you still need to provide it in __VA_ARGS__
 *
 * The GP_FN_PER_* variants ignore the return value of the called function.
 * The GP_FN_RET_PER_* variants "return"s the value returned by the function.
 */

#ifndef GP_FN_PER_BPP_H
#define GP_FN_PER_BPP_H

#include "GP_FnPerBpp.gen.h"

/*
 * Branch on gp_pixmap argument.
 */
#define GP_FN_PER_BPP_PIXMAP(FN_NAME, pixmap, ...) \
	GP_FN_PER_BPP(FN_NAME, (pixmap)->bpp, (pixmap)->bit_endian, __VA_ARGS__)

/*
 * Branch on gp_pixel_type argument.
 */
#define GP_FN_PER_BPP_PIXELTYPE(FN_NAME, type, ...) \
	GP_FN_PER_BPP(FN_NAME, gp_pixel_types[type].size, gp_pixel_types[type].bit_endian, __VA_ARGS__)
/*
 * Branch on gp_pixmap argument.
 */
#define GP_FN_RET_PER_BPP_PIXMAP(FN_NAME, pixmap, ...) \
	GP_FN_RET_PER_BPP(FN_NAME, (pixmap)->bpp, (pixmap)->bit_endian, __VA_ARGS__)

/*
 * Branch on gp_pixel_type argument.
 */
#define GP_FN_RET_PER_BPP_PIXELTYPE(FN_NAME, type, ...) \
	GP_FN_RET_PER_BPP(FN_NAME, gp_pixel_types[type].size, gp_pixel_types[type].bit_endian, __VA_ARGS__)

/*
 * Macros that gets MACRO template for drawing function and generates drawing
 * functions for each BPP.
 *
 * This functions are later used by GP_FN_PER_BPP_PIXMAP() to generate one
 * drawing function for all BPP Yay!
 */
#define GP_DEF_DRAW_FN_PER_BPP(fname, MACRO_NAME) \
	GP_DEF_FN_PER_BPP(fname, MACRO_NAME, gp_putpixel_raw_clipped_)

#define GP_DEF_FILL_FN_PER_BPP(fname, MACRO_NAME) \
	GP_DEF_FN_PER_BPP(fname, MACRO_NAME, gp_hline_raw_)

#define GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, bpp) \
	MACRO_NAME(fname##_##bpp, gp_pixmap *, gp_pixel, fdraw##bpp)

#endif /* GP_FN_PER_BPP_H */
