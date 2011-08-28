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
 * Macros that gets MACRO template for drawing function and generates drawing
 * functions for each BPP.
 *
 * This functions are later used by GP_FN_PER_BPP_CONTEXT() to generate one
 * drawing function for all BPP Yay!
 */
#ifndef GP_DEF_FN_PER_BPP_H
#define GP_DEF_FN_PER_BPP_H

#define GP_DEF_DRAW_FN_PER_BPP(fname, MACRO_NAME) \
	GP_DEF_FN_PER_BPP(fname, MACRO_NAME, GP_PutPixel_Raw_Clipped_)

#define GP_DEF_FILL_FN_PER_BPP(fname, MACRO_NAME) \
	GP_DEF_FN_PER_BPP(fname, MACRO_NAME, GP_HLine_Raw_)

#define GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, bpp) \
	MACRO_NAME(fname##_##bpp, GP_Context *, GP_Pixel, fdraw##bpp)

#define GP_DEF_FN_PER_BPP(fname, MACRO_NAME, fdraw) \
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, 1BPP_LE) \
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, 1BPP_BE) \
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, 2BPP_LE) \
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, 2BPP_BE) \
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, 4BPP_LE) \
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, 4BPP_BE) \
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, 8BPP)    \
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, 16BPP)   \
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, 24BPP)   \
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, 32BPP)   \

/*
 * Dtto for filters.
 *
 * Filter is functions that works on Context per pixel.
 */
#define GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, bpp) \
	MACRO_NAME(fname##_##bpp, GP_Context *, GP_Pixel, \
	           GP_PutPixel_Raw_##bpp, GP_GetPixel_Raw_##bpp)

#define GP_DEF_FFN_PER_BPP(fname, MACRO_NAME) \
	GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, 1BPP_LE) \
	GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, 1BPP_BE) \
	GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, 2BPP_LE) \
	GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, 2BPP_BE) \
	GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, 4BPP_LE) \
	GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, 4BPP_BE) \
	GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, 8BPP)    \
	GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, 16BPP)   \
	GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, 24BPP)   \
	GP_DEF_FFN_FOR_BPP(fname, MACRO_NAME, 32BPP)   \

#endif /* GP_DEF_FN_PER_BPP_H */
