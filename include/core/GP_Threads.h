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

 /*

   A code to ease multithreaded filters and more.

  */

#ifndef GP_THREADS_H
#define GP_THREADS_H

#include <pthread.h>

#include "GP_ProgressCallback.h"
#include "GP_Types.h"

/*
 * Returns an optimal number of threads for a given image size on a particular
 * machine. Most of the time, if the image is not too small, this function
 * returns number of processors as seen by the operating system.
 */
unsigned int GP_NrThreads(GP_Size w, GP_Size h);


/*
 * Multithreaded progress callback priv data guarded by a mutex.
 */
struct GP_ProgressCallbackMPPriv {
	float max;
	int abort;
	pthread_mutex_t mutex;
	GP_ProgressCallback *orig_callback;
};

/*
 * Creates and initalizes a on-the-stack progress callback
 *
 * The intended usage is:
 *
 * GP_PROGRESS_CALLBACK(callback_mp, orig_callback);
 *
 * ...
 *
 * for n threads:
 * 	run_filter(..., callback ? &callback_mp : NULL);
 */
#define GP_PROGRESS_CALLBACK_MP(name, callback)                        \
	struct GP_ProgressCallbackMPPriv name_priv = {                 \
		.max = 0,                                              \
		.abort = 0,                                            \
		.mutex = PTHREAD_MUTEX_INITIALIZER,                    \
		.orig_callback = callback,                             \
	};                                                             \
	GP_PROGRESS_CALLBACK(name, GP_ProgressCallbackMP, &name_priv);

/*
 * Multithreaded callback function itself.
 */
int GP_ProgressCallbackMP(GP_ProgressCallback *self);

#endif /* GP_THREADS_H */
