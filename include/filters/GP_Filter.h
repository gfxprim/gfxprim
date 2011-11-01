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

  Common filters typedefs and includes.

 */

#ifndef GP_FILTERS_FILTER_H
#define GP_FILTERS_FILTER_H

#include "core/GP_Context.h"

/*
 * Progress callback
 *
 * Non zero return value from callback will abort current operation
 * free memory and return NULL from filter.
 */
typedef struct GP_ProgressCallback {
	float percentage;
	int (*callback)(struct GP_ProgressCallback *self);
	void *priv;
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

#endif /* FILTERS_GP_FILTER_H */
