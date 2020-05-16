//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_PROXY_CONN_H__
#define GP_PROXY_CONN_H__

/*
 * Creates a UNIX socket and starts listening for connections.
 *
 * @conn_path Path to an UNIX socket, pass NULL for default.
 * @return A file descriptor.
 */
int gp_proxy_server_init(const char *conn_path);

/*
 * Connects to an UNIX sockets.
 *
 * @conn_path Path to an UNIX socket, pass NULL for default.
 * @return A file descriptor.
 */
int gp_proxy_client_connect(const char *conn_path);

#endif /* GP_PROXY_CONN_H__ */
