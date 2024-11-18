//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @brief A proxy backend connection.
 * @file gp_proxy_conn.h
 */

#ifndef GP_PROXY_CONN_H
#define GP_PROXY_CONN_H

/**
 * @brief Start a proxy backend sever.
 *
 * Creates a UNIX socket and starts listening for application connections.
 *
 * @param conn_path Path to an UNIX socket, pass NULL for default.
 * @return A file descriptor.
 */
int gp_proxy_server_init(const char *conn_path);

/**
 * @brief Connects to a proxy backend sever.
 *
 * Connects to an UNIX socket. This is a low level API the gp_proxy_init() is
 * build on.
 *
 * @param conn_path Path to an UNIX socket, pass NULL for default.
 * @return A file descriptor.
 */
int gp_proxy_client_connect(const char *conn_path);

#endif /* GP_PROXY_CONN_H */
