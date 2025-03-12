//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include <core/gp_debug.h>

#include <backends/gp_proxy_conn.h>

#define CONN_PATH "/tmp/.gpwm"

int gp_proxy_server_init(const char *path)
{
	int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
	struct sockaddr_un addr;

	if (fd < 0)
		return -1;

	if (!path)
		path = CONN_PATH;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	//TODO: bounds check
	memcpy(addr.sun_path, path, sizeof(CONN_PATH));
	unlink(CONN_PATH);

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr))) {
		GP_WARN("Failed to bind '" CONN_PATH "': %s", strerror(errno));
		goto fail;
	}

	if (listen(fd, 5)) {
		GP_WARN("Failed to listen: %s", strerror(errno));
		goto fail;
	}

	setenv("GP_PROXY_PATH", path, 1);

	return fd;
fail:
	close(fd);
	return -1;
}

int gp_proxy_client_connect(const char *path)
{
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un addr;

	if (fd < 0)
		return -1;

	if (!path)
		path = CONN_PATH;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	memcpy(addr.sun_path, CONN_PATH, sizeof(CONN_PATH));

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr))) {
		GP_WARN("Failed to connect '" CONN_PATH ": %s", strerror(errno));
		close(fd);
		return -1;
	}

	return fd;
}
