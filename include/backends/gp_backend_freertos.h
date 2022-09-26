// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BACKENDS_GP_BACKEND_FREERTOS_H
#define BACKENDS_GP_BACKEND_FREERTOS_H

#include <backends/gp_backend.h>

gp_backend *gp_backend_rtos_init(gp_size w, gp_size h, gp_pixel pixel_type, void *fb);

#endif /* BACKENDS_GP_BACKEND_FREERTOS_H */
