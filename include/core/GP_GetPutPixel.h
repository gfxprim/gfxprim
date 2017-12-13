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
 * Copyright (C) 2011-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef CORE_GP_GETPUTPIXEL_H
#define CORE_GP_GETPUTPIXEL_H

#include "GP_Pixmap.h"
#include "GP_Transform.h"
#include "GP_FnPerBpp.h"
#include "GP_Pixel.h"

/*
 * Generated header
 */
#include "GP_GetPutPixel.gen.h"

/*
 * GetPixel with pixmap transformations and clipping.
 * Returns 0 for clipped pixels or pixels outside bitmap.
 */
gp_pixel gp_getpixel(const gp_pixmap *pixmap, gp_coord x, gp_coord y);

/*
 * Version of GetPixel without transformations nor border checking.
 */
static inline gp_pixel gp_getpixel_raw(const gp_pixmap *pixmap,
                                       gp_coord x, gp_coord y)
{
	GP_FN_RET_PER_BPP(gp_getpixel_raw, pixmap->bpp, pixmap->bit_endian,
		pixmap, x, y);

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
	GP_FN_PER_BPP(gp_putpixel_raw, pixmap->bpp, pixmap->bit_endian,
		pixmap, x, y, p);
}

/*
 * Version of PutPixel without transformation and with border checking.
 */
static inline void gp_putpixel_raw_clipped(gp_pixmap *pixmap,
                                           gp_coord x, gp_coord y, gp_pixel p)
{
	GP_FN_PER_BPP(gp_putpixel_raw_clipped, pixmap->bpp, pixmap->bit_endian,
	              pixmap, x, y, p);
}

/*
 * Returns pixel offset.
 */
uint8_t gp_pixel_addr_offset(gp_coord x, gp_pixel_type pixel_type);

#endif /* CORE_GP_GETPUTPIXEL_H */
