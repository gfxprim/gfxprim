// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2011-2023 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef CORE_GP_GETPUTPIXEL_H
#define CORE_GP_GETPUTPIXEL_H

#include <core/gp_pixmap.h>
#include <core/gp_transform.h>
#include <core/gp_pixel.h>
#include <core/gp_pixel_pack.gen.h>
#include <core/gp_get_put_pixel.gen.h>

/*
 * get_pixel with pixmap transformations and clipping.
 * Returns 0 for clipped pixels or pixels outside bitmap.
 */
gp_pixel gp_getpixel(const gp_pixmap *pixmap, gp_coord x, gp_coord y);

/*
 * Version of GetPixel without transformations nor border checking.
 */
static inline gp_pixel gp_getpixel_raw(const gp_pixmap *pixmap,
                                       gp_coord x, gp_coord y)
{
	GP_FN_RET_PER_PACK_PIXMAP(gp_getpixel_raw, pixmap, pixmap, x, y);

	GP_ABORT("Invalid pixmap pixel type");
}

/*
 * Version of GetPixel without transformations and with border checking.
 */
static inline gp_pixel gp_getpixel_raw_clipped(const gp_pixmap *pixmap,
                                               gp_coord x, gp_coord y)
{
	if (GP_PIXEL_IS_CLIPPED(pixmap, x, y))
		return 0;

	return gp_getpixel_raw(pixmap, x, y);
}

/*
 * PutPixel with pixmap transformations and clipping.
 * NOP for clipped pixels or pixels outside bitmap.
 */
void gp_putpixel(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_pixel p);

/*
 * Version of PutPixel without transformations nor border checking.
 */
static inline void gp_putpixel_raw(gp_pixmap *pixmap,
                                   gp_coord x, gp_coord y, gp_pixel p)
{
	GP_FN_PER_PACK_PIXMAP(gp_putpixel_raw, pixmap, pixmap, x, y, p);
}

/*
 * Version of PutPixel without transformation and with border checking.
 */
static inline void gp_putpixel_raw_clipped(gp_pixmap *pixmap,
                                           gp_coord x, gp_coord y, gp_pixel p)
{
	GP_FN_PER_PACK_PIXMAP(gp_putpixel_raw_clipped, pixmap, pixmap, x, y, p);
}

/*
 * Returns pixel offset, in pixels.
 */
uint8_t gp_pixel_addr_offset(const gp_pixmap *pixmap, gp_coord x);

#endif /* CORE_GP_GETPUTPIXEL_H */
