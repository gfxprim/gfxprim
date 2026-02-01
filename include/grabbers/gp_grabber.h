// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_grabber.h
 * @brief A video grabbers.
 */
#ifndef GP_GRABBERS_GRABBER_H
#define GP_GRABBERS_GRABBER_H

#include <core/gp_types.h>
#include <core/gp_compiler.h>

typedef struct gp_grabber gp_grabber;

/** @brief A grabber. */
struct gp_grabber {
	/** @brief Pixmap with current frame. */
	struct gp_pixmap *frame;

	/**
	 * @brief A grabber polling function.
	 *
	 * This function can be either called in a loop.
	 *
	 * @param self A grabber.
	 * @return Zero if there is non new frame and 1 otherwise.
	 */
	int (*poll)(gp_grabber *self);

	/** @brief Grabber fd suitable for polling. */
	int fd;

	/**
	 * @brief Starts the actuall grabbing.
	 *
	 * @param self A grabber.
	 *
	 * May be NULL if not needed.
	 */
	int (*start)(gp_grabber *self);

	/**
	 * @brief Stops the grabbing.
	 *
	 * @param self A grabber.
	 *
	 * May be NULL if not needed.
	 */
	int (*stop)(gp_grabber *self);

	/**
	 * @brief Destroys grabber.
	 *
	 * @param self A grabber.
	 *
	 * Closes fd, frees memory, etc.
	 */
	void (*exit)(gp_grabber *self);

	char priv[] GP_ALIGNED;
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
