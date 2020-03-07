// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <math.h>

#include <core/gp_debug.h>

#include <filters/gp_linear.h>
#include <filters/gp_linear_threads.h>

#include <filters/gp_blur.h>

static inline unsigned int gaussian_kernel_size(float sigma)
{
	int center = 3 * sigma;

	return 2 * center + 1;
}

static inline float gaussian_kernel_init(float sigma, float *kernel)
{
	int i, center = 3 * sigma;
	int N = 2 * center + 1;
	float ret = 0;

	double sigma2 = sigma * sigma;

	for (i = 0; i < N; i++) {
		double r = center - i;
		kernel[i] = exp(-0.5 * (r * r) / sigma2);
		ret += kernel[i];
	}

	return ret;
}

static int gaussian_callback_horiz(gp_progress_cb *self)
{
	gp_progress_cb *callback = self->priv;

	callback->percentage = self->percentage / 2;
	return callback->callback(callback);
}

static int gaussian_callback_vert(gp_progress_cb *self)
{
	gp_progress_cb *callback = self->priv;

	callback->percentage = self->percentage / 2 + 50;
	return callback->callback(callback);
}

int gp_filter_gaussian_blur_raw(const gp_pixmap *src,
                                gp_coord x_src, gp_coord y_src,
                                gp_size w_src, gp_size h_src,
			        gp_pixmap *dst,
                                gp_coord x_dst, gp_coord y_dst,
                                float x_sigma, float y_sigma,
                                gp_progress_cb *callback)
{
	unsigned int size_x = gaussian_kernel_size(x_sigma);
	unsigned int size_y = gaussian_kernel_size(y_sigma);
	const gp_pixmap *tmp = dst;

	GP_DEBUG(1, "Gaussian blur x_sigma=%2.3f y_sigma=%2.3f kernel %ix%i image %ux%u",
	            x_sigma, y_sigma, size_x, size_y, w_src, h_src);

	gp_progress_cb *new_callback = NULL;

	gp_progress_cb gaussian_callback = {
		.callback = gaussian_callback_horiz,
		.priv = callback
	};

	if (callback != NULL)
		new_callback = &gaussian_callback;

	/* compute kernel and apply in horizontal direction */
	if (x_sigma > 0) {
		float kernel_x[size_x];
		float sum = gaussian_kernel_init(x_sigma, kernel_x);

		gp_convolution_params params = {
			.src = src,
			.x_src = x_src,
			.y_src = y_src,
			.w_src = w_src,
			.h_src = h_src,
			.dst = dst,
			.x_dst = x_dst,
			.y_dst = y_dst,
			.kernel = kernel_x,
			.kw = size_x,
			.kh = 1,
			.kern_div = sum,
			.callback = new_callback,
		};

		if (gp_filter_hconvolution_mp_raw(&params))
			return 1;
	} else {
		tmp = src;
	}

	if (new_callback != NULL)
		new_callback->callback = gaussian_callback_vert;

	/*
	 * TODO: This part runs always in-place, which is wrong if
	 *       we run in multiple threads as this part would run
	 *       only singlethreaded. We need temp buffer for the
	 *       first part in this case.
	 */
	/* compute kernel and apply in vertical direction */
	if (y_sigma > 0) {
		float kernel_y[size_y];
		float sum = gaussian_kernel_init(y_sigma, kernel_y);

		gp_convolution_params params = {
			.src = tmp,
			.x_src = x_src,
			.y_src = y_src,
			.w_src = w_src,
			.h_src = h_src,
			.dst = dst,
			.x_dst = x_dst,
			.y_dst = y_dst,
			.kernel = kernel_y,
			.kw = 1,
			.kh = size_y,
			.kern_div = sum,
			.callback = new_callback,
		};

		if (gp_filter_vconvolution_mp_raw(&params))
			return 1;
	}

	gp_progress_cb_done(callback);
	return 0;
}

int gp_filter_gaussian_blur_ex(const gp_pixmap *src,
                               gp_coord x_src, gp_coord y_src,
                               gp_size w_src, gp_size h_src,
                               gp_pixmap *dst,
                               gp_coord x_dst, gp_coord y_dst,
                               float x_sigma, float y_sigma,
                               gp_progress_cb *callback)
{
	GP_CHECK(src->pixel_type == dst->pixel_type);

	/* Check that destination is large enough */
	GP_CHECK(x_dst + (gp_coord)w_src <= (gp_coord)dst->w);
	GP_CHECK(y_dst + (gp_coord)h_src <= (gp_coord)dst->h);

	return gp_filter_gaussian_blur_raw(src, x_src, y_src, w_src, h_src,
	                                   dst, x_dst, y_dst,
	                                   x_sigma, y_sigma, callback);
}

gp_pixmap *gp_filter_gaussian_blur_ex_alloc(const gp_pixmap *src,
                                             gp_coord x_src, gp_coord y_src,
                                             gp_size w_src, gp_size h_src,
				             float x_sigma, float y_sigma,
                                             gp_progress_cb *callback)
{
	gp_pixmap *dst = gp_pixmap_alloc(w_src, h_src, src->pixel_type);

	if (dst == NULL)
		return NULL;

	if (gp_filter_gaussian_blur_raw(src, x_src, y_src, w_src, h_src, dst,
	                                0, 0, x_sigma, y_sigma, callback)) {
		gp_pixmap_free(dst);
		return NULL;
	}

	return dst;
}
