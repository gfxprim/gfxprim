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

#ifndef GP_FILTER_H
#define GP_FILTER_H

#include "core/GP_Context.h"

/*
 * Progress callback
 */
typedef struct GP_ProgressCallback {
	float percentage;
	void (*callback)(struct GP_ProgressCallback *self);
	void *priv;
} GP_ProgressCallback;

static inline void GP_ProgressCallbackReport(GP_ProgressCallback *callback,
                                             float percentage)
{
	if (callback == NULL)
		return;

	callback->percentage = percentage;
	callback->callback(callback);
}

static inline void GP_ProgressCallbackDone(GP_ProgressCallback *callback)
{
	GP_ProgressCallbackReport(callback, 100);
}

#endif /* GP_FILTER_H */
