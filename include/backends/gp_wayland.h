// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>,
 * Copyright (C) 2023 Jiri Dluhos <jiri.bluebear.dluhos@gmail.com>
 */

#ifndef BACKENDS_GP_WAYLAND_H
#define BACKENDS_GP_WAYLAND_H

#include <backends/gp_backend.h>

gp_backend *gp_wayland_init(const char *display,
                            gp_size w, gp_size h, const char *caption);

#endif /* BACKENDS_GP_WAYLAND_H */
