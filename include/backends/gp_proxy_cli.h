//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @brief Proxy sever client (application) helpers.
 * @file gp_proxy_cli.h
 *
 * This header implements helpers for server to communicate with the clients
 * (applications).
 */

#ifndef GP_PROXY_CLI_H
#define GP_PROXY_CLI_H

#include <utils/gp_list.h>
#include <utils/gp_poll.h>
#include <backends/gp_proxy_proto.h>

struct gp_event;

/**
 * @brief A proxy client (application).
 */
typedef struct gp_proxy_cli {
	/**
	 * @brief A sever connection file descriptor.
	 */
	gp_fd fd;

	/** @brief An application name as set by #GP_PROXY_NAME message. */
	char *name;

	gp_dlist_head head;

	/** @brief Connection buffer. */
	gp_proxy_buf buf;
} gp_proxy_cli;

/**
 * @brief Sends a message to a client (application).
 *
 * @param self A client.
 * @param type A message type.
 * @param payload An optional payload. See enum gp_proxy_msg_types.
 *
 * @return Zero on success, non-zero on a failure.
 */
static inline int gp_proxy_cli_send(gp_proxy_cli *self,
                                    enum gp_proxy_msg_types type,
                                    void *payload)
{
	return gp_proxy_send(self->fd.fd, type, payload);
}

/**
 * @brief Starts an application rendering into a SHM buffer.
 *
 * This is a shorthand to:
 *
 * - mapping SHM into memory
 * - setting up a SHM pixmap
 * - setting initial cursor position
 * - starting application rendering
 *
 * @param self A client (application).
 * @param shm A SHM pixmap to start rendering into.
 * @param cur_pos An initial cursor position.
 */
static inline void gp_proxy_cli_show(gp_proxy_cli *self, gp_proxy_shm *shm, gp_proxy_coord *cur_pos)
{
	if (!self)
		return;

	/* Map SHM and create pixmap */
	gp_proxy_cli_send(self, GP_PROXY_MAP, &shm->path);
	gp_proxy_cli_send(self, GP_PROXY_PIXMAP, &shm->pixmap);
	/* Set the current cursor position */
	gp_proxy_cli_send(self, GP_PROXY_CURSOR_POS, cur_pos);
	/* And finally show the app */
	gp_proxy_cli_send(self, GP_PROXY_SHOW, NULL);
}

/**
 * @brief Stops an application rendering and unmaps SHM buffer.
 *
 * This is an shorthand to:
 *
 * - stopping application rendering
 * - unampping a SHM buffer
 *
 * @param self A client (application).
 */
static inline void gp_proxy_cli_hide(gp_proxy_cli *self)
{
	if (!self)
	return;

	gp_proxy_cli_send(self, GP_PROXY_HIDE, NULL);
	gp_proxy_cli_send(self, GP_PROXY_UNMAP, NULL);
}

/**
 * @brief Sends an input event message to the application.
 *
 * @param self A client (application).
 * @param ev An input event.
 */
static inline void gp_proxy_cli_event(gp_proxy_cli *self, gp_event *ev)
{
	if (gp_proxy_send(self->fd.fd, GP_PROXY_EVENT, ev))
		GP_WARN("Dropping event");
}

struct gp_proxy_cli_ops {
	void (*update)(gp_proxy_cli *self, gp_coord x, gp_coord y, gp_size w, gp_size h);
	void (*on_unmap)(gp_proxy_cli *self);
	void (*on_map)(gp_proxy_cli *self);
	void (*on_hide)(gp_proxy_cli *self);
	void (*on_show)(gp_proxy_cli *self);
};

/*
 * Has to be called when there are data ready at cli->fd.
 *
 * @self Pointer to a client.
 * @return Zero on success, non-zero otherwise.
 */
int gp_proxy_cli_read(gp_proxy_cli *self, struct gp_proxy_cli_ops *ops);

/*
 * Adds a new client to the clients list pointed by root pointer.
 *
 * @root Root of double linked list of connected clients.
 * @cli_fd Client file descriptor.
 * @return A pointer to a newly allocated client.
 */
gp_proxy_cli *gp_proxy_cli_add(gp_dlist *clients, int cli_fd);

/*
 * Removes client from a list of clients.
 *
 * @root Root of double linked list of connected clients.
 * @self A client to be removed.
 */
void gp_proxy_cli_rem(gp_dlist *clients, gp_proxy_cli *self);

#endif /* GP_PROXY_CLI_H */
