// SPDX-License-Identifier: LGPL-2.1-or-later
/*

  Copyright (C) 2019-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @brief A proxy backend.
 * @file gp_proxy.h
 *
 * Proxy backend allows to run an application in a different process while
 * events are routed over socket and graphics is rendered into a shared memory.
 *
 * Apart from isolation this also allows multiplexing input device and screen
 * between different applications on the top of another backend, e.g.
 * DRM or a Framebuffer.
 */

#ifndef GP_PROXY_H
#define GP_PROXY_H

#include <backends/gp_backend.h>

/**
 * @brief Connects to a proxy backend.
 *
 * Conects to a proxy backend and returns a backend handle.
 *
 * @param path Path to an UNIX socket, pass NULL for default.
 * @param name An application name.
 */
gp_backend *gp_proxy_init(const char *path, const char *title);

#endif /* GP_PROXY_H */
