@ include source.t
/*
 * Horizontal Mirror alogorithm
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_debug.h>
#include <filters/gp_rotate.h>

@ for ps in pixelsizes:
static int mirror_h_raw_{{ ps.suffix }}(const gp_pixmap *src,
                                        gp_pixmap *dst,
                                        gp_progress_cb *callback)
{
	uint32_t x, y;
	gp_pixel tmp;

	GP_DEBUG(1, "Mirroring image %ux%u horizontally", src->w, src->h);

	for (x = 0; x < src->w/2; x++) {
		uint32_t xm = src->w - x - 1;
		for (y = 0; y < src->h; y++) {
			tmp = gp_getpixel_raw_{{ ps.suffix }}(src, x, y);
			gp_putpixel_raw_{{ ps.suffix }}(dst, x, y, gp_getpixel_raw_{{ ps.suffix }}(src, xm, y));
			gp_putpixel_raw_{{ ps.suffix }}(dst, xm, y, tmp);
		}

		if (gp_progress_cb_report(callback, 2 * x, src->w, src->h)) {
			errno = ECANCELED;
			return 1;
		}
	}

	/* Copy the middle odd line */
	if (src != dst && src->w % 2) {
		x = src->w / 2;
		for (y = 0; y < src->h; y++)
			gp_putpixel_raw_{{ ps.suffix }}(dst, x, y, gp_getpixel_raw_{{ ps.suffix }}(src, x, y));
	}

	gp_progress_cb_done(callback);
	return 0;
}

@ end
@
static int gp_filter_mirror_h_raw(const gp_pixmap *src, gp_pixmap *dst,
                                  gp_progress_cb *callback)
{
	GP_FN_RET_PER_BPP_PIXMAP(mirror_h_raw, src, src, dst, callback);
	return 1;
}

int gp_filter_mirror_h(const gp_pixmap *src, gp_pixmap *dst,
                       gp_progress_cb *callback)
{
	GP_ASSERT(src->pixel_type == dst->pixel_type,
	          "The src and dst pixel types must match");
	GP_ASSERT(src->w <= dst->w && src->h <= dst->h,
	          "Destination is not large enough");

	if (gp_filter_mirror_h_raw(src, dst, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}

gp_pixmap *gp_filter_mirror_h_alloc(const gp_pixmap *src,
                                    gp_progress_cb *callback)
{
	gp_pixmap *res;

	res = gp_pixmap_copy(src, 0);

	if (res == NULL)
		return NULL;

	if (gp_filter_mirror_h_raw(src, res, callback)) {
		gp_pixmap_free(res);
		return NULL;
	}

	return res;
}
