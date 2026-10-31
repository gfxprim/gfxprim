// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef PIXMAP_GEN_H
#define PIXMAP_GEN_H

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>

#include "tst_test.h"

/* Matches function used to generate input images for loader tests. */
static inline uint8_t pat(unsigned int x, unsigned int y, unsigned int c)
{
	return (x * x * 7 + y * y * 11 + x * y * 5 + x * 29 + y * 47 + c * 83) & 0xff;
}

static inline gp_pixel exp_rgb(gp_coord x, gp_coord y)
{
	return GP_PIXEL_CREATE_RGB888(pat(x, y, 0), pat(x, y, 1), pat(x, y, 2));
}

static inline gp_pixel exp_gray(gp_coord x, gp_coord y)
{
	return pat(x, y, 0);
}

static inline gp_pixel exp_gray4(gp_coord x, gp_coord y)
{
	return (pat(x, y, 0) >> 4) * 17;
}

static inline uint8_t clamp8(double v)
{
	long r = lround(v);

	return r < 0 ? 0 : (r > 255 ? 255 : r);
}

static inline gp_pixel exp_sycc(gp_coord x, gp_coord y)
{
	double l = pat(x, y, 0);
	double u = pat(x, y, 1) - 128.0;
	double v = pat(x, y, 2) - 128.0;

	return GP_PIXEL_CREATE_RGB888(clamp8(l + 1.402 * v),
	                              clamp8(l - 0.344136 * u - 0.714136 * v),
	                              clamp8(l + 1.772 * u));
}

static inline gp_pixel exp_cmyk(gp_coord x, gp_coord y)
{
	return GP_PIXEL_CREATE_CMYK8888(pat(x, y, 3), pat(x, y, 2),
	                                pat(x, y, 1), pat(x, y, 0));
}

static inline int tst_pixmap_check(const gp_pixmap *pixmap,
                                   gp_size w, gp_size h,
                                   gp_pixel_type pixel_type,
                                   gp_pixel (*fn)(gp_coord x, gp_coord y))
{
	gp_coord x, y;

	if (pixmap->w != w || pixmap->h != h || pixmap->pixel_type != pixel_type) {
		tst_msg("Got %ux%u %s expected %ux%u %s", pixmap->w, pixmap->h,
		        gp_pixel_type_name(pixmap->pixel_type), w, h,
		        gp_pixel_type_name(pixel_type));
		return TST_FAILED;
	}

	for (y = 0; y < (gp_coord)h; y++) {
		for (x = 0; x < (gp_coord)w; x++) {
			gp_pixel p = gp_getpixel_raw(pixmap, x, y);
			gp_pixel e = fn(x, y);

			if (p != e) {
				tst_msg("Pixel %ix%i is 0x%08x expected 0x%08x",
				        (int)x, (int)y, p, e);
				return TST_FAILED;
			}
		}
	}

	return TST_PASSED;
}

#endif /* PIXMAP_GEN_H */
