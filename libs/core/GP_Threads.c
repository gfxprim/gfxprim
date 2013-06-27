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
#include <stdlib.h>

#include "GP_Common.h"
#include "GP_Debug.h"

#include "GP_Threads.h"

static unsigned int nr_threads = 1;

unsigned int GP_NrThreads(GP_Size w, GP_Size h, GP_ProgressCallback *callback)
{
	int count, threads;
	char *env;

	/* Try to override nr_threads from the callback first */
	if (callback != NULL && callback->threads) {
		GP_DEBUG(1, "Overriding nr_threads from callback to %i",
		         callback->threads);
		nr_threads = callback->threads;
	} else {
		/* Then try to override it from the enviroment variable */
		env = getenv("GP_THREADS");

		if (env) {
			nr_threads = atoi(env);
			GP_DEBUG(1, "Using GP_THREADS=%u from enviroment "
			            "variable", nr_threads);
		}
	}

	if (nr_threads == 0) {
		count = sysconf(_SC_NPROCESSORS_ONLN);
		GP_DEBUG(1, "Found %i CPUs", count);
	} else {
		count = nr_threads;
		GP_DEBUG(1, "Using nr_threads=%i", count);
	}

	threads = GP_MIN(count, (int)(w * h / 1024) + 1);

	/* Call to the sysconf may return -1 if unsupported */
	if (threads < -1)
		threads = 1;

	GP_DEBUG(1, "Max threads %i image size %ux%u runnig %u threads",
	         count, w, h, threads);

	return threads;
}

void GP_NrThreadsSet(unsigned int nr)
{
	nr_threads = nr;

	GP_DEBUG(1, "Setting default number of threads to %u", nr);
}

int GP_ProgressCallbackMP(GP_ProgressCallback *self)
{
	struct GP_ProgressCallbackMPPriv *priv = self->priv;

	/*
	 * If any thread got non-zero return value from a callback abort all.
	 */
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
