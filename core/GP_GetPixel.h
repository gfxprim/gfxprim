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

#ifndef GP_GETPIXEL_H
#define GP_GETPIXEL_H

#include "GP_Context.h"

#define GP_GETPIXEL_1BPP(context, x, y) ({ \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, x, y); \
	(0x80>>(x%8) & *gp_pix_addr) ? 1 : 0; \
})

#define GP_GETPIXEL_2BPP(context, x, y) ({ \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, x, y); \
	(*gp_pix_addr & (0xc0 >> (2*(x%4)))) >> (2*(3 - x%4)); \
})

#define GP_GETPIXEL_4BPP(contex, x, y) ({ \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, x, y); \
	(*gp_pix_addr & (0xf0 >> (4*(x%2)))) >> (4*(1 - x%2)); \
})	

#define GP_GETPIXEL_8BPP(contex, x, y) ({ \
	*GP_PIXEL_ADDR(context, x, y); \
})

#define GP_GETPIXEL_16BPP(contex, x, y) ({ \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, x, y); \
	gp_pix_addr[0]<<8 | gp_pix_addr[1]; \
})

#define GP_GETPIXEL_24BPP(contex, x, y) ({ \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, x, y); \
	gp_pix_addr[0]<<16 | gp_pix_addr[1] << 8 | \
	gp_pix_addr[2]; \
})

#define GP_GETPIXEL_32BPP(contex, x, y) ({ \
	uint8_t *gp_pix_addr = GP_PIXEL_ADDR(context, x, y); \
	gp_pix_addr[0]<<24 | gp_pix_addr[1] << 16 | \
	gp_pix_addr[2] << 8 | gp_pix_addr[3]; \
})

/*
 * General getpixel. 
 */
GP_Pixel GP_GetPixel(GP_Context *context, int x, int y);

/*
 * General rotated getpixel. 
 */
GP_Pixel GP_TGetPixel(GP_Context *context, int x, int y);

#endif /* GP_GETPIXEL_H */
