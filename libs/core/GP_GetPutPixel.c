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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 * Copyright (C) 2012      Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "GP_GetPutPixel.h"
#include "GP_Transform.h"

gp_pixel gp_getpixel(const gp_pixmap *pixmap, gp_coord x, gp_coord y)
{
	GP_TRANSFORM_POINT(pixmap, x, y);
	if (GP_PIXEL_IS_CLIPPED(pixmap, x, y))
		return 0;
	return gp_getpixel_raw(pixmap, x, y);
}

void gp_putpixel(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_pixel p)
{
	GP_TRANSFORM_POINT(pixmap, x, y);
	if (!GP_PIXEL_IS_CLIPPED(pixmap, x, y))
		gp_putpixel_raw(pixmap, x, y, p);
}

uint8_t gp_pixel_addr_offset(gp_coord x, gp_pixel_type pixel_type)
{
	GP_FN_RET_PER_BPP_PIXELTYPE(GP_PIXEL_ADDR_OFFSET, pixel_type, x);

	return 0;
}
