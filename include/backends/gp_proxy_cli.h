//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_PROXY_CLI_H
#define GP_PROXY_CLI_H

#include <utils/gp_poll.h>
#include <backends/gp_proxy_proto.h>

struct gp_event;

struct gp_proxy_cli {
	gp_fd fd;

	char *name;

	/* Double linked list of clients */
	struct gp_proxy_cli *next;
	struct gp_proxy_cli *prev;

	/* Connection buffer */
	struct gp_proxy_buf buf;
};

static inline void gp_proxy_cli_show(struct gp_proxy_cli *self)
{
        if (!self)
                return;

        gp_proxy_send(self->fd.fd, GP_PROXY_SHOW, NULL);
}

static inline void gp_proxy_cli_hide(struct gp_proxy_cli *self)
{
        if (!self)
                return;

        gp_proxy_send(self->fd.fd, GP_PROXY_HIDE, NULL);
}

static inline void gp_proxy_cli_event(struct gp_proxy_cli *self, gp_event *ev)
{
        if (gp_proxy_send(self->fd.fd, GP_PROXY_EVENT, ev))
                GP_WARN("Dropping event");
}

static inline int gp_proxy_cli_send(struct gp_proxy_cli *self,
                                    enum gp_proxy_msg_types type,
                                    void *payload)
{
	return gp_proxy_send(self->fd.fd, type, payload);
}

struct gp_proxy_cli_ops {
	void (*update)(struct gp_proxy_cli *self, gp_coord x, gp_coord y, gp_size w, gp_size h);
	void (*on_unmap)(struct gp_proxy_cli *self);
	void (*on_map)(struct gp_proxy_cli *self);
	void (*on_hide)(struct gp_proxy_cli *self);
	void (*on_show)(struct gp_proxy_cli *self);
};

/*
 * Has to be called when there are data ready at cli->fd.
 *
 * @self Pointer to a client.
 * @return Zero on success, non-zero otherwise.
 */
int gp_proxy_cli_read(struct gp_proxy_cli *self, struct gp_proxy_cli_ops *ops);

/*
 * Adds a new client to the clients list pointed by root pointer.
 *
 * @root Root of double linked list of connected clients.
 * @cli_fd Client file descriptor.
 * @return A pointer to a newly allocated client.
 */
struct gp_proxy_cli *gp_proxy_cli_add(struct gp_proxy_cli **root, int cli_fd);

/*
 * Removes client from a list of clients.
 *
 * @root Root of double linked list of connected clients.
 * @self A client to be removed.
 */
void gp_proxy_cli_rem(struct gp_proxy_cli **root, struct gp_proxy_cli *self);


#endif /* GP_PROXY_CLI_H */
