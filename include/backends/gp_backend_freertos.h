// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_backend_freertos.h
 * @brief A work in progress freertos backend.
 */

#ifndef BACKENDS_GP_BACKEND_FREERTOS_H
#define BACKENDS_GP_BACKEND_FREERTOS_H

#include <backends/gp_backend.h>

/**
 * @brief Initializes freertos backend.
 *
 * The backend expects tinyusb hid library is linked with the application and
 * uses the tinyusb hid callbacks to generate input events.
 *
 * See libs/backends/freertos/ for details.
 *
 * @param w A buffer width.
 * @param h A buffer height.
 * @param pixel_type A buffer pixel_type.
 * @param fb A pointer to a start of a buffer.
 *
 * @return Newly initialized backend.
 */
gp_backend *gp_backend_rtos_init(gp_size w, gp_size h, gp_pixel pixel_type, void *fb);

#endif /* BACKENDS_GP_BACKEND_FREERTOS_H */
