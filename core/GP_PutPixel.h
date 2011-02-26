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

#ifndef GP_PUTPIXEL_H
#define GP_PUTPIXEL_H

#include "GP_Context.h"

/*
 * Putpixel macros.
 */
#define GP_PUTPIXEL_1BPP(context, x, y, pixel) do { \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, y, x); \
\
	if (pixel) \
		*gp_pix_addr |=  (0x80>>(x%8)); \
	else  \
		*gp_pix_addr &= ~(0x80>>(x%8)); \
} while (0)

#define GP_PUTPIXEL_2BPP(context, x, y, pixel) do { \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, y, x); \
\
	*gp_pix_addr = (*gp_pix_addr & ~(0xc0 >> (2*(x%4)))) | \
	               (pixel << (2*(3 - x%4))); \
\
} while (0)

#define GP_PUTPIXEL_4BPP(context, x, y, pixel) do { \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, y, x); \
\
	*gp_pix_addr = (*gp_pix_addr & (0xf0 >> (4*(x%2)))) | \
	               (pixel << (4*(1 - x%2))); \
} while (0)

#define GP_PUTPIXEL_8BPP(context, x, y, pixel) do { \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, y, x); \
\
	*gp_pix_addr = pixel & 0xff; \
} while (0)

#define GP_PUTPIXEL_16BPP(context, x, y, pixel) do { \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, y, x); \
\
	gp_pix_addr[0] = pixel      & 0xff; \
	gp_pix_addr[1] = (pixel>>8) & 0xff; \
} while (0)

#define GP_PUTPIXEL_24BPP(context, x, y, pixel) do { \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, y, x); \
\
	gp_pix_addr[0] = pixel       & 0xff; \
	gp_pix_addr[1] = (pixel>>8)  & 0xff; \
	gp_pix_addr[2] = (pixel>>16) & 0xff; \
} while (0)

#define GP_PUTPIXEL_32BPP(context, x, y, pixel) do { \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, y, x); \
\
	gp_pix_addr[0] = pixel       & 0xff; \
	gp_pix_addr[1] = (pixel>>8)  & 0xff; \
	gp_pix_addr[2] = (pixel>>16) & 0xff; \
	gp_pix_addr[3] = (pixel>>24) & 0xff; \
} while (0)

/*
 * Safe functions, that checks clipping.
 */
GP_RetCode GP_PutPixel1bpp(GP_Context *context, int x, int y, GP_Pixel pixel);
GP_RetCode GP_PutPixel2bpp(GP_Context *context, int x, int y, GP_Pixel pixel);
GP_RetCode GP_PutPixel4bpp(GP_Context *context, int x, int y, GP_Pixel pixel);
GP_RetCode GP_PutPixel8bpp(GP_Context *context, int x, int y, GP_Pixel pixel);
GP_RetCode GP_PutPixel16bpp(GP_Context *context, int x, int y, GP_Pixel pixel);
GP_RetCode GP_PutPixel24bpp(GP_Context *context, int x, int y, GP_Pixel pixel);
GP_RetCode GP_PutPixel32bpp(GP_Context *context, int x, int y, GP_Pixel pixel);

/*
 * General putpixel.
 */
GP_RetCode GP_PutPixel(GP_Context *context, int x, int y, GP_Pixel pixel);

/*
 * General rotated putpixel.
 */
GP_RetCode GP_TPutPixel(GP_Context *context, int x, int y, GP_Pixel pixel);

#endif /* GP_PUTPIXEL_H */
