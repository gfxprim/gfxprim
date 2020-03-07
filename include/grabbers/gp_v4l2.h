// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_GRABBERS_V4L2_H
#define GP_GRABBERS_V4L2_H

#include <grabbers/gp_grabber.h>

/*
 * Create V4L2 grabber.
 *
 * The prefered_width and height may not be used if driver does support only
 * fixed image size.
 */
gp_grabber *gp_grabber_v4l2_init(const char *device,
                                 unsigned int preferred_width,
				 unsigned int preferred_height);

#endif /* GP_GRABBERS_V4L2_H */
