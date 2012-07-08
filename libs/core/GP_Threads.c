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

#include "GP_Common.h"
#include "GP_Debug.h"

#include "GP_Threads.h"

unsigned int GP_NrThreads(GP_Size w, GP_Size h)
{
	int count = sysconf(_SC_NPROCESSORS_ONLN);
	int threads = GP_MIN(count, (int)(w * h / 1024) + 1);

	if (count < -1)
		threads = 1;

	GP_DEBUG(1, "Found %u CPUs size %ux%u runnig %u threads",
	         count, w, h, threads);

	return threads;
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
