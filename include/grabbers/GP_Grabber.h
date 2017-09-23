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

#ifndef GP_GRABBERS_GRABBER_H
#define GP_GRABBERS_GRABBER_H

struct GP_Pixmap;

typedef struct GP_Grabber {
	/*
	 * Pixmap with current frame.
	 */
	struct GP_Pixmap *frame;

	/*
	 * Returns 0 if there are no images in queue and 1 otherwise.
	 */
	int (*Poll)(struct GP_Grabber *self);

	/*
	 * Grabber fd, may be set -1 if grabber doesn't have one.
	 */
	int fd;

	/*
	 * Starts the actuall grabbing. May be NULL if not needed.
	 */
	int (*Start)(struct GP_Grabber *self);

	/*
	 * Stops the grabbing. May be NULL if not needed.
	 */
	int (*Stop)(struct GP_Grabber *self);

	/*
	 * Exit functions. Closes fd, frees memory.
	 */
	void (*Exit)(struct GP_Grabber *self);

	char priv[];
} GP_Grabber;

#define GP_GRABBER_PRIV(grabber) ((void*)(grabber)->priv)

static inline void GP_GrabberExit(struct GP_Grabber *self)
{
	self->Exit(self);
}

static inline int GP_GrabberPoll(struct GP_Grabber *self)
{
	return self->Poll(self);
}

static inline int GP_GrabberStart(struct GP_Grabber *self)
{
	if (self->Start)
		return self->Start(self);

	return 0;
}

static inline int GP_GrabberStop(struct GP_Grabber *self)
{
	if (self->Stop)
		return self->Stop(self);

	return 0;
}

#endif /* GP_GRABBERS_GRABBER_H */
