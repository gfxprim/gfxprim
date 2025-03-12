//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <core/gp_debug.h>
#include <backends/gp_proxy_shm.h>
#include <backends/gp_proxy_proto.h>
#include <backends/gp_proxy_cli.h>

static int set_name(struct gp_proxy_cli *app, void *name, size_t size)
{
	char *ptr;

	if (app->name) {
		GP_DEBUG(1, "Name already set!");
		return 1;
	}

	ptr = malloc(size + 1);
	if (!ptr)
		return 1;

	memcpy(ptr, name, size);
	ptr[size] = 0;

	app->name = ptr;

	return 0;
}

void gp_proxy_cli_rem(gp_dlist *clients, struct gp_proxy_cli *self)
{
	GP_DEBUG(1, "Freeing client (%p) fd %i", self, self->fd.fd);

	gp_dlist_rem(clients, &self->head);

	free(self->name);
	free(self);
}

int gp_proxy_cli_read(struct gp_proxy_cli *self)
{
	ssize_t ret;

	ret = gp_proxy_buf_recv(self->fd.fd, &self->buf);

	if (ret > 0) {
		GP_DEBUG(4, "Client (%p) '%s' read %zu bytes",
		         self, self->name, ret);
		return 0;
	}

	if (ret == 0) {
		GP_WARN("Client (%p) '%s' Connection closed", self, self->name);
		return 1;
	}

	if (ret < 0 && errno != EAGAIN) {
		GP_WARN("Client (%p) '%s': Connection error: %s",
		         self, self->name, strerror(errno));
		return 1;
	}


	return 0;
}

int gp_proxy_cli_msg(gp_proxy_cli *self, gp_proxy_msg **rmsg)
{
	int ret;

	ret = gp_proxy_next(&self->buf, rmsg);
	if (ret < 0)
		return 1;

	if (ret == 0)
		return 0;

	gp_proxy_msg *msg = *rmsg;

	switch (msg->type) {
	case GP_PROXY_NAME:
		if (set_name(self, msg->payload, msg->size - 8))
			return 1;

		GP_DEBUG(1, "Client (%p) fd %i name set to '%s'",
		         self, self->fd.fd, self->name);
	break;
	case GP_PROXY_EXIT:
		GP_DEBUG(1, "Client (%p) '%s' fd %i requests exit",
		         self, self->name, self->fd.fd);
	break;
	case GP_PROXY_UPDATE:
		GP_DEBUG(4, "Client (%p) '%s' fd %i requested update %ux%u-%ux%u",
			 self, self->name, self->fd.fd,
		         msg->rect.rect.x, msg->rect.rect.y,
			 msg->rect.rect.w, msg->rect.rect.h);
	break;
	case GP_PROXY_MAP:
		GP_DEBUG(1, "Client (%p) '%s' fd %i mapped buffer",
		         self, self->name, self->fd.fd);
	break;
	case GP_PROXY_UNMAP:
		GP_DEBUG(1, "Client (%p) '%s' fd %i unmapped buffer",
		         self, self->name, self->fd.fd);
	break;
	case GP_PROXY_SHOW:
		GP_DEBUG(1, "Client (%p) '%s' fd %i started rendering",
		         self, self->name, self->fd.fd);
	break;
	case GP_PROXY_HIDE:
		GP_DEBUG(1, "Client (%p) '%s' fd %i stopped rendering",
		         self, self->name, self->fd.fd);
	break;
	default:
		GP_DEBUG(1, "Client (%p) '%s' fd (%i) invalid request %i",
		         self, self->name, self->fd.fd, msg->type);
		return 1;
	}

	return 0;
}

struct gp_proxy_cli *gp_proxy_cli_add(gp_dlist *clients, int cli_fd)
{
	struct gp_proxy_cli *cli = malloc(sizeof(*cli));

	GP_DEBUG(1, "Allocating client (%p) fd %i", cli, cli_fd);

	if (!cli)
		return NULL;

	if (fcntl(cli_fd, F_SETFL, O_NONBLOCK | O_CLOEXEC))
		GP_WARN("Failed to set cli fd non blocking: %s", strerror(errno));

	cli->fd = (gp_fd) {
		.fd = cli_fd,
		.events = GP_POLLIN,
	};

	cli->name = NULL;

	gp_proxy_buf_init(&cli->buf);

	gp_dlist_push_head(clients, &cli->head);

	return cli;
}
