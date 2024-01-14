//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_PROXY_CLI_H
#define GP_PROXY_CLI_H

#include <utils/gp_list.h>
#include <utils/gp_poll.h>
#include <backends/gp_proxy_proto.h>

struct gp_event;

typedef struct gp_proxy_cli {
	gp_fd fd;

	char *name;

	gp_dlist_head head;

	/* Connection buffer */
	gp_proxy_buf buf;
} gp_proxy_cli;

static inline void gp_proxy_cli_show(gp_proxy_cli *self)
{
        if (!self)
                return;

        gp_proxy_send(self->fd.fd, GP_PROXY_SHOW, NULL);
}

static inline void gp_proxy_cli_hide(gp_proxy_cli *self)
{
        if (!self)
                return;

        gp_proxy_send(self->fd.fd, GP_PROXY_HIDE, NULL);
}

static inline void gp_proxy_cli_event(gp_proxy_cli *self, gp_event *ev)
{
        if (gp_proxy_send(self->fd.fd, GP_PROXY_EVENT, ev))
                GP_WARN("Dropping event");
}

static inline int gp_proxy_cli_send(gp_proxy_cli *self,
                                    enum gp_proxy_msg_types type,
                                    void *payload)
{
	return gp_proxy_send(self->fd.fd, type, payload);
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
