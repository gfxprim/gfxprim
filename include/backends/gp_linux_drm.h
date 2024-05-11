// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_linux_drm.h
 * @brief A Linux DRM backend.
 *
 * Simple DRM backend along with Linux input for input devices.
 */
#ifndef BACKENDS_GP_LINUX_DRM_H
#define BACKENDS_GP_LINUX_DRM_H

#include <backends/gp_backend.h>

/** @brief Linux DRM backend flags. */
enum gp_linux_drm_flags {
	/** @brief Do not use linux input for keyboard and mouse */
	GP_LINUX_DRM_NO_INPUT = 0x01,
};

/**
 * @brief A Linux DRM backend initialization.
 *
 * So far single monitor.
 *
 * @param drm_path A path to dri device file e.g. "/dev/dri/card0"
 * @param flags enum gp_linux_drm_flags
 * @return Initialized backend or NULL in a case of a failure.
 */
gp_backend *gp_linux_drm_init(const char *drm_path, int flags);

#endif /* BACKENDS_GP_LINUX_DRM_H */
