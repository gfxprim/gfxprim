// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Catch all header for backends.

 */

#ifndef BACKENDS_GP_BACKENDS_H
#define BACKENDS_GP_BACKENDS_H

/* Base backend definitions */
#include <backends/gp_backend.h>

/* Clipboard */
#include <backends/gp_clipboard.h>

/* Generic backend initalization */
#include <backends/gp_backend_init.h>

/* Backend input drivers */
#include <backends/gp_backend_input.h>

/* Backends. */
#include <backends/gp_linux_fb.h>
#include <backends/gp_linux_drm.h>
#include <backends/gp_wayland.h>
#include <backends/gp_sdl.h>
#include <backends/gp_x11.h>
#include "backends/gp_xcb.h"
#include <backends/gp_aalib.h>
#include <backends/gp_backend_virtual.h>
#include <backends/gp_proxy.h>

#endif /* BACKENDS_GP_BACKENDS_H */
