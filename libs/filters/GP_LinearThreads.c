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

#include "GP_Linear.h"
#include "GP_LinearThreads.h"

static int nr_threads(GP_Size w, GP_Size h)
{
	int count = sysconf(_SC_NPROCESSORS_ONLN);
	int threads = GP_MIN(count, (int)(w * h / 1024) + 1);

	if (count < -1)
		threads = 1;

	GP_DEBUG(1, "Found %u CPUs size %ux%u runnig %u threads",
	         count, w, h, threads);

	return threads;
}

/*
 * Multithreaded callback wrapper.
 */
struct callback_priv {
	float max;
	int abort;
	unsigned int nr_threads;
	pthread_mutex_t mutex;
	GP_ProgressCallback *orig_callback;
};

static int progress_callback_mp(GP_ProgressCallback *self)
{
	struct callback_priv *priv = self->priv;

	/* If any thread got non-zero return value from a callback, abort all */
	if (priv->abort)
		return 1;

	if (pthread_mutex_trylock(&priv->mutex)) {
		GP_DEBUG(1, "Mutex locked, skipping calllback.");
		return 0;
	}

	/* Compute max value for the percentage */
	priv->orig_callback->percentage = GP_MAX(self->percentage, priv->max);
	priv->max = priv->orig_callback->percentage;

	/* Call the original callback */
	int ret = priv->orig_callback->callback(priv->orig_callback);
	
	/* Turn on abort flag if callback returned nonzero */
	if (ret)
		priv->abort = 1;

	pthread_mutex_unlock(&priv->mutex);

	return ret;
}

static void callback_priv_init(struct callback_priv *priv, int nr_threads,
                               GP_ProgressCallback *orig_callback)
{
	priv->nr_threads = nr_threads;
	priv->orig_callback = orig_callback;
	priv->max = 0;
	priv->abort = 0;
	pthread_mutex_init(&priv->mutex, NULL);
}

static void callback_priv_exit(struct callback_priv *priv)
{
	pthread_mutex_destroy(&priv->mutex);
}

/*
 * This code just packs and unpacks convolution parameters.
 */
struct convolution {
	const GP_Context *src;
	GP_Coord x_src;
	GP_Coord y_src;
	GP_Size w_src;
	GP_Size h_src;
	
	GP_Context *dst;
	GP_Coord x_dst;
	GP_Coord y_dst;

	float *kernel;
	unsigned int ks;
	float kern_div;

	GP_ProgressCallback *callback;
};

static void convolution_init(struct convolution *conv, const GP_Context *src,
                             GP_Coord x_src, GP_Coord y_src,
                             GP_Size w_src, GP_Size h_src,
                             GP_Context *dst, GP_Coord x_dst, GP_Coord y_dst,
		             float kernel[], unsigned int ks, float kern_div,
                             GP_ProgressCallback *callback)
{
	conv->src = src;
	conv->x_src = x_src;
	conv->y_src = y_src;
	conv->w_src = w_src;
	conv->h_src = h_src;
	conv->dst = dst;
	conv->x_dst = x_dst;
	conv->y_dst = y_dst;
	conv->kernel = kernel;
	conv->ks = ks;
	conv->kern_div = kern_div;
	conv->callback = callback;
}

static void *h_linear_convolution(void *arg)
{
	struct convolution *conv = arg;

	long ret;

	ret = GP_FilterHLinearConvolution_Raw(conv->src, conv->x_src, conv->y_src,
	                                      conv->w_src, conv->h_src, conv->dst,
					      conv->x_dst, conv->y_dst, conv->kernel,
					      conv->ks, conv->kern_div, conv->callback);
	return (void*)ret;
}

static void *v_linear_convolution(void *arg)
{
	struct convolution *conv = arg;

	long ret;

	ret = GP_FilterVLinearConvolution_Raw(conv->src, conv->x_src, conv->y_src,
	                                      conv->w_src, conv->h_src, conv->dst,
					      conv->x_dst, conv->y_dst, conv->kernel,
					      conv->ks, conv->kern_div, conv->callback);
	return (void*)ret;
}


int GP_FilterHLinearConvolutionMP_Raw(const GP_Context *src,
                                     GP_Coord x_src, GP_Coord y_src,
                                     GP_Size w_src, GP_Size h_src,
                                     GP_Context *dst,
                                     GP_Coord x_dst, GP_Coord y_dst,
				     float kernel[], uint32_t kw, float kern_div,
				     GP_ProgressCallback *callback)
{
	int t = nr_threads(w_src, h_src);

	if (t == 1) {
		return GP_FilterHLinearConvolution_Raw(src, x_src, y_src,
		                                       w_src, h_src,
		                                       dst, x_dst, y_dst,
		                                       kernel, kw, kern_div,
		                                       callback);
	}

	/* Create multithreaded safe callback on the stack */
	struct callback_priv priv;
	callback_priv_init(&priv, t, callback);
	GP_ProgressCallback callback_mp = {0, progress_callback_mp, &priv};

	/* Run t threads */
	int i;
	pthread_t threads[t];
	struct convolution convs[t];
	GP_Size h = h_src/t;

	for (i = 0; i < t; i++) {
		GP_Coord y_src_2 = y_src + i * h;
		GP_Coord y_dst_2 = y_dst + i * h;
		GP_Size  h_src_2 = h;
		
		if (i == t - 1)
			h_src_2 = h_src - i * h;

		/* Pack convolution parameters into the structure */
		convolution_init(&convs[i], src, x_src, y_src_2, w_src, h_src_2,
		                 dst, x_dst, y_dst_2, kernel, kw, kern_div,
				 callback ? &callback_mp : NULL);

		pthread_create(&threads[i], NULL, h_linear_convolution, &convs[i]); 
	}

	int ret = 0;

	for (i = 0; i < t; i++) {
		long r;

		pthread_join(threads[i], (void*)&r);

		ret |= (int)r;
	}
	
	callback_priv_exit(&priv);

	return ret;
}


int GP_FilterVLinearConvolutionMP_Raw(const GP_Context *src,
                                     GP_Coord x_src, GP_Coord y_src,
                                     GP_Size w_src, GP_Size h_src,
                                     GP_Context *dst,
                                     GP_Coord x_dst, GP_Coord y_dst,
                                     float kernel[], uint32_t kh, float kern_div,
                                     GP_ProgressCallback *callback)
{
	int t = nr_threads(w_src, h_src);

	if (t == 1) {
		return GP_FilterVLinearConvolution_Raw(src, x_src, y_src,
		                                       w_src, h_src,
		                                       dst, x_dst, y_dst,
		                                       kernel, kh, kern_div,
		                                       callback);
	}
	
	/* Create multithreaded safe callback on the stack */
	struct callback_priv priv;
	callback_priv_init(&priv, t, callback);
	GP_ProgressCallback callback_mp = {0, progress_callback_mp, &priv};
	
	int i;
	pthread_t threads[t];
	struct convolution convs[t];
	GP_Size h = h_src/t;

	for (i = 0; i < t; i++) {
		GP_Coord y_src_2 = y_src + i * h;
		GP_Coord y_dst_2 = y_dst + i * h;
		GP_Size  h_src_2 = h;
		
		if (i == t - 1)
			h_src_2 = h_src - i * h;
		
		/* Pack convolution parameters into the structure */
		convolution_init(&convs[i], src, x_src, y_src_2, w_src, h_src_2,
		                 dst, x_dst, y_dst_2, kernel, kh, kern_div,
				 callback ? &callback_mp : NULL);
		
		pthread_create(&threads[i], NULL, v_linear_convolution, &convs[i]); 
	}

	int ret = 0;

	for (i = 0; i < t; i++) {
		long r;
		pthread_join(threads[i], (void*)&r);

		ret |= (int)r;
	}
	
	callback_priv_exit(&priv);

	return ret;
}

/*
int GP_FilterLinearConvolutionMP_Raw(const GP_Context *src,
                                    GP_Coord x_src, GP_Coord y_src,
                                    GP_Size w_src, GP_Size h_src,
                                    GP_Context *dst,
                                    GP_Coord x_dst, GP_Coord y_dst,
                                    float kernel[], uint32_t kw, uint32_t kh,
                                    float kern_div, GP_ProgressCallback *callback)
{
}
*/
