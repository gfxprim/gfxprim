// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2019 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BACKENDS_GP_XCB_H
#define BACKENDS_GP_XCB_H

#include <backends/gp_backend.h>

/*
 * Initalize XCB backend.
 *
 * The display may be NULL for default display ($DISPLAY shell variable will
 * be used).
 *
 * The coordinates are position and geometry for newly created window.
 *
 * Upon failure NULL is returned.
 */
gp_backend *gp_xcb_init(const char *display, int x, int y,
                        unsigned int w, unsigned int h,
			const char *caption);

#endif /* BACKENDS_GP_XCB_H */
