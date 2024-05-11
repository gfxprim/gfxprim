// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>,
 * Copyright (C) 2023 Jiri Dluhos <jiri.bluebear.dluhos@gmail.com>
 */

/**
 * @file gp_wayland.h
 * @brief A Wayland backend.
 */
#ifndef BACKENDS_GP_WAYLAND_H
#define BACKENDS_GP_WAYLAND_H

#include <backends/gp_backend.h>

/**
 * @brief A Wayland backend initialization.
 *
 * @param display A Wayland display.
 * @param w A window width.
 * @param h A window height.
 * @param caption A window caption.
 * @return A newly allocated and initialized backend.
 */
gp_backend *gp_wayland_init(const char *display,
                            gp_size w, gp_size h, const char *caption);

#endif /* BACKENDS_GP_WAYLAND_H */
