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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Progress callback implementation.

  Progress callbacks serves following purposes:

  - ability to visibly show algorithm progress
  - ability to correctly abort operation in the middle of processing
  - ability to override default number of threads to use for the operation

 */

#ifndef CORE_GP_PROGRESSCALLBACK_H
#define CORE_GP_PROGRESSCALLBACK_H

/*
 * Progress callback
 *
 * Non zero return value from callback will abort current operation
 * free memory and return NULL from filter/loader...
 */
typedef struct GP_ProgressCallback {
	float percentage;

	int (*callback)(struct GP_ProgressCallback *self);
	void *priv;

	/*
	 * Number of threads to use (if supported). This setting could be used
	 * to override the default number of threads as returned by
	 * GP_NrThreads().
	 *
	 * 0 == use number returned from GP_NrThreads().
	 *
	 *   >= 1 use exactly n threads
	 */
	unsigned int threads;
} GP_ProgressCallback;

static inline int GP_ProgressCallbackReport(GP_ProgressCallback *callback,
                                            unsigned int val, unsigned int max,
					    unsigned int mul __attribute__((unused)))
{
	if (callback == NULL)
		return 0;

	if (val % 100)
		return 0;

	callback->percentage = 100.00 * val / max;
	return callback->callback(callback);
}

static inline void GP_ProgressCallbackDone(GP_ProgressCallback *callback)
{
	if (callback == NULL)
		return;

	callback->percentage = 100;
	callback->callback(callback);
}

#define GP_PROGRESS_CALLBACK(name, pcallback, ppriv) \
	GP_ProgressCallback name = {                 \
		.callback = pcallback,               \
		.priv = ppriv,                       \
	};

#endif /* CORE_GP_PROGRESSCALBACK_H */
