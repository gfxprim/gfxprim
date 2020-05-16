// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

/*
 * Proxy backend allows to run an application in a different process while
 * events are routed over socket and graphics is rendered into a shared memory.
 * Apart from isolation this also allows multiplexing input device and screen
 * between different applications on the top of another backend, e.g.
 * framebuffer.
 */

#ifndef GP_PROXY_H__
#define GP_PROXY_H__

#include <backends/gp_backend.h>

/*
 * Initalize proxy backend.
 *
 * @path Path to an UNIX socket, pass NULL for default.
 * @name Application name.
 */
gp_backend *gp_proxy_init(const char *path, const char *title);

#endif /* GP_PROXY_H__ */
