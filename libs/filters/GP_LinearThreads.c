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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "core/GP_Common.h"
#include "core/GP_Debug.h"
#include "core/GP_Threads.h"

#include "GP_Linear.h"
#include "GP_LinearThreads.h"

static void *h_linear_convolution(void *arg)
{
	struct GP_ConvolutionParams *params = arg;

	long ret = GP_FilterHConvolution_Raw(params);

	return (void*)ret;
}

static void *v_linear_convolution(void *arg)
{
	struct GP_ConvolutionParams *params = arg;

	long ret = GP_FilterVConvolution_Raw(params);

	return (void*)ret;
}

static void *linear_convolution(void *arg)
{
	struct GP_ConvolutionParams *params = arg;

	long ret = GP_FilterConvolution_Raw(params);

	return (void*)ret;
}

int GP_FilterHConvolutionMP_Raw(const GP_ConvolutionParams *params)
{
	int i, t = GP_NrThreads(params->w_src, params->h_src, params->callback);

	if (t == 1)
		return GP_FilterHConvolution_Raw(params);

	if (params->src == params->dst) {
		GP_DEBUG(1, "In-place filter detected, running in one thread.");
		return GP_FilterHConvolution_Raw(params);
	}

	GP_PROGRESS_CALLBACK_MP(callback_mp, params->callback);

	/* Run t threads */
	pthread_t threads[t];
	struct GP_ConvolutionParams convs[t];
	GP_Size h = params->h_src/t;

	for (i = 0; i < t; i++) {
		GP_Coord y_src_2 = params->y_src + i * h;
		GP_Coord y_dst_2 = params->y_dst + i * h;
		GP_Size  h_src_2 = h;

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


int GP_FilterVConvolutionMP_Raw(const GP_ConvolutionParams *params)
{
	int i, t = GP_NrThreads(params->w_src, params->h_src, params->callback);

	if (t == 1)
		return GP_FilterVConvolution_Raw(params);

	if (params->src == params->dst) {
		GP_DEBUG(1, "In-place filter detected, running in one thread.");
		return GP_FilterVConvolution_Raw(params);
	}

	GP_PROGRESS_CALLBACK_MP(callback_mp, params->callback);

	/* Run t threads */
	pthread_t threads[t];
	struct GP_ConvolutionParams convs[t];
	GP_Size h = params->h_src/t;

	for (i = 0; i < t; i++) {
		GP_Coord y_src_2 = params->y_src + i * h;
		GP_Coord y_dst_2 = params->y_dst + i * h;
		GP_Size  h_src_2 = h;

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


int GP_FilterConvolutionMP_Raw(const GP_ConvolutionParams *params)
{
	int i, t = GP_NrThreads(params->w_src, params->h_src, params->callback);

	if (t == 1)
		return GP_FilterConvolution_Raw(params);

	if (params->src == params->dst) {
		GP_DEBUG(1, "In-place filter detected, running in one thread.");
		return GP_FilterConvolution_Raw(params);
	}

	GP_PROGRESS_CALLBACK_MP(callback_mp, params->callback);

	/* Run t threads */
	pthread_t threads[t];
	struct GP_ConvolutionParams convs[t];
	GP_Size h = params->h_src/t;

	for (i = 0; i < t; i++) {
		GP_Coord y_src_2 = params->y_src + i * h;
		GP_Coord y_dst_2 = params->y_dst + i * h;
		GP_Size  h_src_2 = h;

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
