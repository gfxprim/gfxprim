// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2012      Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_transform.h>

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
