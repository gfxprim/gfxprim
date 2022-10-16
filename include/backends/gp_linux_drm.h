// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BACKENDS_GP_LINUX_DRM_H
#define BACKENDS_GP_LINUX_DRM_H

#include <backends/gp_backend.h>

/**
 * @drm_path A path to dri device file e.g. "/dev/dri/card0"
 * @flags Currently unused.
 */
gp_backend *gp_linux_drm_init(const char *drm_path, int flags);

#endif /* BACKENDS_GP_LINUX_DRM_H */
