// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_GRABBERS_GRABBER_H
#define GP_GRABBERS_GRABBER_H

#include <core/gp_types.h>

typedef struct gp_grabber gp_grabber;

struct gp_grabber {
	/*
	 * Pixmap with current frame.
	 */
	struct gp_pixmap *frame;

	/*
	 * Returns 0 if there are no images in queue and 1 otherwise.
	 */
	int (*poll)(gp_grabber *self);

	/*
	 * Grabber fd, may be set -1 if grabber doesn't have one.
	 */
	int fd;

	/*
	 * starts the actuall grabbing. May be NULL if not needed.
	 */
	int (*start)(gp_grabber *self);

	/*
	 * stops the grabbing. May be NULL if not needed.
	 */
	int (*stop)(gp_grabber *self);

	/*
	 * exit functions. Closes fd, frees memory.
	 */
	void (*exit)(gp_grabber *self);

	char priv[];
};

#define GP_GRABBER_PRIV(grabber) ((void*)(grabber)->priv)

static inline void gp_grabber_exit(gp_grabber *self)
{
	self->exit(self);
}

static inline int gp_grabber_poll(gp_grabber *self)
{
	return self->poll(self);
}

static inline int gp_grabber_start(gp_grabber *self)
{
	if (self->start)
		return self->start(self);

	return 0;
}

static inline int gp_grabber_stop(gp_grabber *self)
{
	if (self->stop)
		return self->stop(self);

	return 0;
}

#endif /* GP_GRABBERS_GRABBER_H */
