// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "core/gp_common.h"
#include <core/gp_debug.h>
#include <core/gp_threads.h>

#include <filters/gp_linear.h>
#include <filters/gp_linear_threads.h>

static void *h_linear_convolution(void *arg)
{
	struct gp_convolution_params *params = arg;

	long ret = gp_filter_hconvolution_raw(params);

	return (void*)ret;
}

static void *v_linear_convolution(void *arg)
{
	struct gp_convolution_params *params = arg;

	long ret = gp_filter_vconvolution_raw(params);

	return (void*)ret;
}

static void *linear_convolution(void *arg)
{
	struct gp_convolution_params *params = arg;

	long ret = gp_filter_convolution_raw(params);

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

	int ret = 0;

	for (i = 0; i < t; i++) {
		long r;

		pthread_join(threads[i], (void*)&r);

		ret |= (int)r;
	}

	return ret;
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

	int ret = 0;

	for (i = 0; i < t; i++) {
		long r;
		pthread_join(threads[i], (void*)&r);

		ret |= (int)r;
	}

	return ret;
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

	int ret = 0;

	for (i = 0; i < t; i++) {
		long r;
		pthread_join(threads[i], (void*)&r);

		ret |= (int)r;
	}

	return ret;
}
