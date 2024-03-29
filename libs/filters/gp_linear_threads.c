// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_PTHREAD
# include <pthread.h>
# include <core/gp_threads.h>
#endif

#include <core/gp_common.h>
#include <core/gp_debug.h>

#include <filters/gp_linear.h>
#include <filters/gp_linear_threads.h>

#ifdef HAVE_PTHREAD

static void *h_linear_convolution(void *arg)
{
	struct gp_convolution_params *params = arg;
	long ret = 0;

	if (gp_filter_hconvolution_raw(params))
		ret = errno;

	return (void*)ret;
}

static void *v_linear_convolution(void *arg)
{
	struct gp_convolution_params *params = arg;
	long ret = 0;

	if (gp_filter_vconvolution_raw(params))
		ret = errno;

	return (void*)ret;
}

static void *linear_convolution(void *arg)
{
	struct gp_convolution_params *params = arg;
	long ret = 0;

	if (gp_filter_convolution_raw(params))
		ret = errno;

	return (void*)ret;
}

int gp_filter_hconvolution_mp_raw(const gp_convolution_params *params)
{
	int i, t = gp_nr_threads(params->w_src, params->h_src,
				 params->callback);

	if (t == 1)
		return gp_filter_hconvolution_raw(params);

	if (params->src == params->dst) {
		GP_DEBUG(1, "In-place filter detected, running in one thread.");
		return gp_filter_hconvolution_raw(params);
	}

	GP_PROGRESS_CALLBACK_MP(callback_mp, params->callback);

	/* Run t threads */
	pthread_t threads[t];
	struct gp_convolution_params convs[t];
	gp_size h = params->h_src/t;

	for (i = 0; i < t; i++) {
		gp_coord y_src_2 = params->y_src + i * h;
		gp_coord y_dst_2 = params->y_dst + i * h;
		gp_size  h_src_2 = h;

		if (i == t - 1)
			h_src_2 = params->h_src - i * h;

		convs[i] = *params;
		convs[i].y_src = y_src_2;
		convs[i].h_src = h_src_2;
		convs[i].y_dst = y_dst_2;
		convs[i].callback = params->callback ? &callback_mp : NULL;
		pthread_create(&threads[i], NULL, h_linear_convolution, &convs[i]);
	}

	int err = 0;

	for (i = 0; i < t; i++) {
		long r;
		pthread_join(threads[i], (void*)&r);

		if (r)
			err = r;
	}

	if (err) {
		errno = err;
		return -1;
	}

	return 0;
}


int gp_filter_vconvolution_mp_raw(const gp_convolution_params *params)
{
	int i, t = gp_nr_threads(params->w_src, params->h_src,
				 params->callback);

	if (t == 1)
		return gp_filter_vconvolution_raw(params);

	if (params->src == params->dst) {
		GP_DEBUG(1, "In-place filter detected, running in one thread.");
		return gp_filter_vconvolution_raw(params);
	}

	GP_PROGRESS_CALLBACK_MP(callback_mp, params->callback);

	/* Run t threads */
	pthread_t threads[t];
	struct gp_convolution_params convs[t];
	gp_size h = params->h_src/t;

	for (i = 0; i < t; i++) {
		gp_coord y_src_2 = params->y_src + i * h;
		gp_coord y_dst_2 = params->y_dst + i * h;
		gp_size  h_src_2 = h;

		if (i == t - 1)
			h_src_2 = params->h_src - i * h;

		convs[i] = *params;
		convs[i].y_src = y_src_2;
		convs[i].h_src = h_src_2;
		convs[i].y_dst = y_dst_2;
		convs[i].callback = params->callback ? &callback_mp : NULL;

		pthread_create(&threads[i], NULL, v_linear_convolution, &convs[i]);
	}

	int err = 0;

	for (i = 0; i < t; i++) {
		long r;
		pthread_join(threads[i], (void*)&r);

		if (r)
			err = r;
	}

	if (err) {
		errno = err;
		return -1;
	}

	return 0;
}


int gp_filter_convolution_mp_raw(const gp_convolution_params *params)
{
	int i, t = gp_nr_threads(params->w_src, params->h_src,
				 params->callback);

	if (t == 1)
		return gp_filter_convolution_raw(params);

	if (params->src == params->dst) {
		GP_DEBUG(1, "In-place filter detected, running in one thread.");
		return gp_filter_convolution_raw(params);
	}

	GP_PROGRESS_CALLBACK_MP(callback_mp, params->callback);

	/* Run t threads */
	pthread_t threads[t];
	struct gp_convolution_params convs[t];
	gp_size h = params->h_src/t;

	for (i = 0; i < t; i++) {
		gp_coord y_src_2 = params->y_src + i * h;
		gp_coord y_dst_2 = params->y_dst + i * h;
		gp_size  h_src_2 = h;

		if (i == t - 1)
			h_src_2 = params->h_src - i * h;

		convs[i] = *params;
		convs[i].y_src = y_src_2;
		convs[i].h_src = h_src_2;
		convs[i].y_dst = y_dst_2;
		convs[i].callback = params->callback ? &callback_mp : NULL;

		pthread_create(&threads[i], NULL, linear_convolution, &convs[i]);
	}

	int err = 0;

	for (i = 0; i < t; i++) {
		long r;
		pthread_join(threads[i], (void*)&r);

		if (r)
			err = r;
	}

	if (err) {
		errno = err;
		return -1;
	}

	return 0;
}

#else

int gp_filter_hconvolution_mp_raw(const gp_convolution_params *params)
{
	return gp_filter_hconvolution_raw(params);
}

int gp_filter_vconvolution_mp_raw(const gp_convolution_params *params)
{
	return gp_filter_vconvolution_raw(params);
}

int gp_filter_convolution_mp_raw(const gp_convolution_params *params)
{
	return gp_filter_convolution_raw(params);
}

#endif /* HAVE_PTHREAD */
