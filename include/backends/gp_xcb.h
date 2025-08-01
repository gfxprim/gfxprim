// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2019 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_xcb.h
 * @brief A xcb backend.
 */

#ifndef BACKENDS_GP_XCB_H
#define BACKENDS_GP_XCB_H

#include <backends/gp_backend.h>

/**
 * @brief Initalize XCB backend.
 *
 * The display may be NULL for default display ($DISPLAY shell variable will
 * be used).
 *
 * @param display A X display to connect to, pass NULL for a default.
 * @param x A x window offset.
 * @param y A y window offset.
 * @param w A window width.
 * @param h A window height.
 *
 * @return Newly initialized backend or on a failure NULL.
 */
gp_backend *gp_xcb_init(const char *display, int x, int y,
                        unsigned int w, unsigned int h,
			const char *caption);

#endif /* BACKENDS_GP_XCB_H */
