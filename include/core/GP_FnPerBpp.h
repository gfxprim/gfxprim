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
 * GP_Context or given GP_PixelType.
 *
 * Extra arguments are arguments to be passed to the function.
 * Note that if the function takes the context/type/bpp as an argument, 
 * you still need to provide it in __VA_ARGS__
 *
 * The GP_FN_PER_* variants ignore the return value of the called function.
 * The GP_FN_RET_PER_* variants "return"s the value returned by the function.
 */

#ifndef GP_FN_PER_BPP_H
#define GP_FN_PER_BPP_H

#include "GP_FnPerBpp.gen.h"

/*
 * Branch on GP_Context argument. 
 */
#define GP_FN_PER_BPP_CONTEXT(FN_NAME, context, ...) \
	GP_FN_PER_BPP(FN_NAME, (context)->bpp, (context)->bit_endian, __VA_ARGS__)

/*
 * Branch on GP_PixelType argument. 
 */
#define GP_FN_PER_BPP_PIXELTYPE(FN_NAME, type, ...) \
	GP_FN_PER_BPP(FN_NAME, GP_PixelTypes[type].size, GP_PixelTypes[type].bit_endian, __VA_ARGS__)
/*
 * Branch on GP_Context argument. 
 */
#define GP_FN_RET_PER_BPP_CONTEXT(FN_NAME, context, ...) \
	GP_FN_RET_PER_BPP(FN_NAME, (context)->bpp, (context)->bit_endian, __VA_ARGS__)

/*
 * Branch on GP_PixelType argument. 
 */
#define GP_FN_RET_PER_BPP_PIXELTYPE(FN_NAME, type, ...) \
	GP_FN_RET_PER_BPP(FN_NAME, GP_PixelTypes[type].size, GP_PixelTypes[type].bit_endian, __VA_ARGS__)

/*
 * Macros that gets MACRO template for drawing function and generates drawing
 * functions for each BPP.
 *
 * This functions are later used by GP_FN_PER_BPP_CONTEXT() to generate one
 * drawing function for all BPP Yay!
 */
#define GP_DEF_DRAW_FN_PER_BPP(fname, MACRO_NAME) \
	GP_DEF_FN_PER_BPP(fname, MACRO_NAME, GP_PutPixel_Raw_Clipped_)

#define GP_DEF_FILL_FN_PER_BPP(fname, MACRO_NAME) \
	GP_DEF_FN_PER_BPP(fname, MACRO_NAME, GP_HLine_Raw_)

#define GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, bpp) \
	MACRO_NAME(fname##_##bpp, GP_Context *, GP_Pixel, fdraw##bpp)

/*
 * Dtto for filters.
 *
 * Filter is functions that works on Context per pixel.
 */
#define GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, bpp) \
	MACRO_NAME(fname##_##bpp, GP_Context *, GP_Pixel, \
	           GP_PutPixel_Raw_##bpp, GP_GetPixel_Raw_##bpp)

#endif /* GP_FN_PER_BPP_H */
