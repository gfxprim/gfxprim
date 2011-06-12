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
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
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
 * Branch on bpp and bit_endian. 
 */
#define GP_FN_PER_BPP(FN_NAME, bpp, bit_endian, ...) \
\
	switch (bpp) { \
	case 1: \
		{\
		if (bit_endian == GP_BIT_ENDIAN_LE)\
			FN_NAME##_1BPP_LE(__VA_ARGS__);\
		else \
			FN_NAME##_1BPP_BE(__VA_ARGS__);\
		} \
	break; \
	case 2: \
		{\
		if (bit_endian == GP_BIT_ENDIAN_LE)\
			FN_NAME##_2BPP_LE(__VA_ARGS__);\
		else \
			FN_NAME##_2BPP_BE(__VA_ARGS__);\
		} \
	break; \
	case 4: \
		{\
		if (bit_endian == GP_BIT_ENDIAN_LE)\
			FN_NAME##_4BPP_LE(__VA_ARGS__);\
		else \
			FN_NAME##_4BPP_BE(__VA_ARGS__);\
		} \
	break; \
	case 8: \
		FN_NAME##_8BPP(__VA_ARGS__); \
	break; \
	case 16: \
		FN_NAME##_16BPP(__VA_ARGS__); \
	break; \
	case 24: \
		FN_NAME##_24BPP(__VA_ARGS__); \
	break; \
	case 32: \
		FN_NAME##_32BPP(__VA_ARGS__); \
	break; \
	default: \
	break; \
	} \

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
 * Branch on bpp and bit_endian. 
 */
#define GP_FN_RET_PER_BPP(FN_NAME, bpp, bit_endian, ...) \
\
	switch (bpp) { \
	case 1: \
		{\
		if (bit_endian == GP_BIT_ENDIAN_LE)\
			return FN_NAME##_1BPP_LE(__VA_ARGS__);\
		else \
			return FN_NAME##_1BPP_BE(__VA_ARGS__);\
		} \
	case 2: \
		{\
		if (bit_endian == GP_BIT_ENDIAN_LE)\
			return FN_NAME##_2BPP_LE(__VA_ARGS__);\
		else \
			return FN_NAME##_2BPP_BE(__VA_ARGS__);\
		} \
	case 4: \
		{\
		if (bit_endian == GP_BIT_ENDIAN_LE)\
			return FN_NAME##_4BPP_LE(__VA_ARGS__);\
		else \
			return FN_NAME##_4BPP_BE(__VA_ARGS__);\
		} \
	case 8: \
		return FN_NAME##_8BPP(__VA_ARGS__); \
	case 16: \
		return FN_NAME##_16BPP(__VA_ARGS__); \
	case 24: \
		return FN_NAME##_24BPP(__VA_ARGS__); \
	case 32: \
		return FN_NAME##_32BPP(__VA_ARGS__); \
	}
