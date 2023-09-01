//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <string.h>
#include <sys/epoll.h>

#include <core/gp_debug.h>
#include <utils/gp_poll.h>

void gp_poll_clear(gp_poll *self)
{
	GP_DEBUG(2, "Clearing all fds");

	if (self->ep_fd > 0)
		close(self->ep_fd);
}

int gp_poll_add(gp_poll *self, gp_fd *fd)
{
	GP_DEBUG(2, "Adding fd %i event %p priv %p", fd->fd, fd->event, fd->priv);

	if (self->ep_fd <= 0) {
		GP_DEBUG(3, "Opening epoll instance");
		self->ep_fd = epoll_create(1);
		if (self->ep_fd < 0) {
			GP_WARN("epoll_create() failed: %s", strerror(errno));
			return 1;
		}
	}

	struct epoll_event ev = {
		.data = {.ptr = fd},
		.events = fd->events,
	};

	epoll_ctl(self->ep_fd, EPOLL_CTL_ADD, fd->fd, &ev);

	gp_dlist_push_head(&self->fds, &fd->lhead);

	return 0;
}

int gp_poll_rem(gp_poll *self, gp_fd *fd)
{
	GP_DEBUG(2, "Removing fd %i event %p priv %p", fd->fd, fd->event, fd->priv);

	struct epoll_event ev = {
		.data = {.ptr = fd},
		.events = fd->events,
	};

	epoll_ctl(self->ep_fd, EPOLL_CTL_DEL, fd->fd, &ev);

	gp_dlist_rem(&self->fds, &fd->lhead);

	if (!self->fds.cnt) {
		GP_DEBUG(3, "Closing epoll instance");
		close(self->ep_fd);
	}

	return 0;
}

gp_fd *gp_poll_rem_by_fd(gp_poll *self, int fd)
{
	gp_fd *ret;
	gp_dlist_head *i;

	GP_LIST_FOREACH(&self->fds, i) {
		ret = GP_LIST_ENTRY(i, gp_fd, lhead);

		if (ret->fd == fd)
			break;
	}

	if (i) {
		gp_poll_rem(self, ret);
		return ret;
	}

	return NULL;
}

int gp_poll_wait(gp_poll *self, int timeout)
{
	struct epoll_event events[16];
	int nfds, i;

	nfds = epoll_wait(self->ep_fd, events, GP_ARRAY_SIZE(events), timeout);
	for (i = 0; i < nfds; i++) {
		gp_fd *fd = events[i].data.ptr;

		fd->revents = events[i].events;

		if (fd->event(fd))
			gp_poll_rem(self, fd);
	}

	return 0;
}
