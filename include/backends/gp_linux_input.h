// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
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
 * @backend A backend to registred the handler for
 * @return Returns zero on success non-zero otherwise.
 */
int gp_linux_input_hotplug_new(gp_backend *backend);

int gp_linux_input_new(const char *dev_path, gp_backend *backend);

#endif /* BACKENDS_GP_LINUX_INPUT_H__ */
