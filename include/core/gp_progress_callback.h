// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_progress_callback.h
 * @brief A progress callback.
 *
 * Progress callbacks serves following purposes:
 *
 * - ability to visibly show algorithm progress
 * - ability to abort operation in the middle of processing
 * - ability to override default number of threads to use for the operation
 */

#ifndef CORE_GP_PROGRESS_CALLBACK_H
#define CORE_GP_PROGRESS_CALLBACK_H

#include <core/gp_types.h>

/**
 * @brief Progress callback.
 *
 * Non zero return value from callback will abort current operation
 * free memory and return NULL from filter/loader...
 */
struct gp_progress_cb {
	/** @brief Current progress */
	float percentage;

	/** @brief A progress callback handler */
	int (*callback)(struct gp_progress_cb *self);
	/** @brief User private pointer */
	void *priv;

	/**
	 * @brief A number of threads.
	 *
	 * Number of threads to use (if supported). This setting could be used
	 * to override the default number of threads as returned by
	 * gp_nr_threads().
	 *
	 * 0 == use number returned from gp_nr_threads().
	 *
	 *   >= 1 use exactly n threads
	 */
	unsigned int threads;
};

/**
 * @brief Reports progress.
 *
 * This function is called by image loaders and filters to report progress.
 *
 * The progress is reported as percentage based on the val where max is 100%.
 *
 * @param callback A progress callback, passed from application.
 * @param val Current value.
 * @param max A maximal value.
 */
static inline int gp_progress_cb_report(gp_progress_cb *callback,
                                        unsigned int val, unsigned int max,
					unsigned int mul __attribute__((unused)))
{
	if (!callback)
		return 0;

	if (val % 100)
		return 0;

	callback->percentage = 100.00 * val / max;
	return callback->callback(callback);
}

static inline void gp_progress_cb_done(gp_progress_cb *callback)
{
	if (!callback)
		return;

	callback->percentage = 100;
	callback->callback(callback);
}

#define GP_PROGRESS_CALLBACK(name, pcallback, ppriv) \
	gp_progress_cb name = {                      \
		.callback = pcallback,               \
		.priv = ppriv,                       \
	};

#endif /* CORE_GP_PROGRESS_CALBACK_H */
