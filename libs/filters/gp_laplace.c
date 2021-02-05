// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_debug.h>
#include <core/gp_get_put_pixel.h>
#include <filters/gp_linear.h>
#include <filters/gp_laplace.h>

int gp_filter_laplace(const gp_pixmap *src, gp_pixmap *dst,
		      gp_progress_cb *callback)
{
	GP_DEBUG(1, "Laplace filter %ux%u", src->w, src->h);

	float kern[9] = {0,  1,  0,
	                 1, -4,  1,
	                 0,  1,  0};

	if (gp_filter_linear_convolution_raw(src, 0, 0, src->w, src->h,
	                                     dst, 0, 0, kern, 3, 3, 1, callback))
		return 1;

	return 0;
}

gp_pixmap *gp_filter_laplace_alloc(const gp_pixmap *src,
                                   gp_progress_cb *callback)
{
	gp_pixmap *ret = gp_pixmap_copy(src, 0);

	if (ret == NULL)
		return NULL;

	if (gp_filter_laplace(src, ret, callback)) {
		gp_pixmap_free(ret);
		return NULL;
	}

	return ret;
}


int gp_filter_edge_sharpening(const gp_pixmap *src, gp_pixmap *dst,
                              float w, gp_progress_cb *callback)
{
	/* Identity kernel */
	float kern[9] = {0,  0,  0,
	                 0,  1,  0,
	                 0,  0,  0};

	GP_DEBUG(1, "Laplace Edge Sharpening filter %ux%u w=%f",
	         src->w, src->h, w);

	/* Create combined kernel */
	kern[1] -=  1.00 * w;
	kern[3] -=  1.00 * w;
	kern[4] -= -4.00 * w;
	kern[5] -=  1.00 * w;
	kern[7] -=  1.00 * w;

	if (gp_filter_linear_convolution_raw(src, 0, 0, src->w, src->h,
	                                     dst, 0, 0,  kern, 3, 3, 1, callback))
		return 1;

	return 0;
}

gp_pixmap *gp_filter_edge_sharpening_alloc(const gp_pixmap *src, float w,
                                           gp_progress_cb *callback)
{
	gp_pixmap *ret = gp_pixmap_copy(src, 0);

	if (ret == NULL)
		return NULL;

	if (gp_filter_edge_sharpening(src, ret, w, callback)) {
		gp_pixmap_free(ret);
		return NULL;
	}

	return ret;
}
