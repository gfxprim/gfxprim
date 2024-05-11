// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_linux_input.h
 * @brief Linux input drivers.
 */

#ifndef BACKENDS_GP_LINUX_INPUT_H__
#define BACKENDS_GP_LINUX_INPUT_H__

#include <backends/gp_backend.h>
#include <backends/gp_backend_input.h>

/**
 * @brief Registers a hotplug handler for the Linux input devices
 *
 * Hotplug handler for the /dev/input/device* devices is registered first, then
 * all currently existing devices are walked and passed to the
 * gp_linux_input_new() function.
 *
 * This is called by backend drivers that use linux input, e.g. linux drm
 * backend and there is no need to call this function from the application.
 *
 * @param backend A backend to registred the handler for
 * @return Returns zero on success non-zero otherwise.
 */
int gp_linux_input_hotplug_new(gp_backend *backend);

/**
 * @brief Creates a new linux input driver and attaches it to the backend.
 *
 * This functions is usually called by the hotplug input driver attached to the
 * backend by the gp_linux_input_hotplug_new() call.
 *
 * @param dev_path A path to a linux input device, e.g. /dev/input/event10
 * @param backend A backend to attach the input driver to.
 * @return Returns zero on success non-zero otherwise.
 */
int gp_linux_input_new(const char *dev_path, gp_backend *backend);

#endif /* BACKENDS_GP_LINUX_INPUT_H__ */
