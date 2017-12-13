@ include source.t
/*
 * Nearest Neighbour resampling
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include <core/GP_Pixmap.h>
#include <core/GP_GetPutPixel.h>
#include <core/GP_Debug.h>
#include <filters/GP_ResizeNN.h>

@ for pt in pixeltypes:
@     if not pt.is_unknown():
static int resize_nn_{{ pt.name }}(const gp_pixmap *src, gp_pixmap *dst,
                                   gp_progress_cb *callback)
{
	uint32_t xmap[dst->w];
	uint32_t ymap[dst->h];
	uint32_t i;
	gp_coord x, y;

	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	/* Pre-compute mapping for interpolation */
	for (i = 0; i < dst->w; i++)
		xmap[i] = ((((i * (src->w - 1))<<8)) / (dst->w - 1) + (1<<7))>>8;

	for (i = 0; i < dst->h; i++)
		ymap[i] = ((((i * (src->h - 1))<<8) + (dst->h - 1)/2) / (dst->h - 1) + (1<<7))>>8;

	/* Interpolate */
	for (y = 0; y < (gp_coord)dst->h; y++) {
		for (x = 0; x < (gp_coord)dst->w; x++) {
			gp_pixel pix = gp_getpixel_raw_{{ pt.pixelsize.suffix }}(src, xmap[x], ymap[y]);

			gp_putpixel_raw_{{ pt.pixelsize.suffix }}(dst, x, y, pix);
		}

		if (gp_progress_cb_report(callback, y, dst->h, dst->w)) {
			errno = ECANCELED;
			return 1;
		}
	}

	gp_progress_cb_done(callback);
	return 0;
}

@ end
@
static int resize_nn(const gp_pixmap *src, gp_pixmap *dst,
                     gp_progress_cb *callback)
{
	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		return resize_nn_{{ pt.name }}(src, dst, callback);
	break;
@ end
	default:
		return -1;
	}
}

int gp_filter_resize_nn(const gp_pixmap *src, gp_pixmap *dst,
                        gp_progress_cb *callback)
{
	if (src->pixel_type != dst->pixel_type) {
		GP_WARN("The src and dst pixel types must match");
		errno = EINVAL;
		return 1;
	}

	return resize_nn(src, dst, callback);
}
