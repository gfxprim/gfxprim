//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <poll.h>
#include <core/gp_debug.h>
#include <utils/gp_vec.h>
#include <utils/gp_fds.h>

void gp_fds_clear(struct gp_fds *self)
{
	GP_DEBUG(2, "Clearing all fds");
	gp_vec_free(self->fds);
	gp_vec_free(self->pfds);
}

int gp_fds_add(struct gp_fds *self, int fd, short events,
               int (*event)(struct gp_fd *self, struct pollfd *pfd), void *priv)
{
	GP_DEBUG(2, "Adding fd %i event %p priv %p", fd, event, priv);

	if (!self->fds) {
		self->fds = gp_vec_new(1, sizeof(*self->fds));
		self->pfds = gp_vec_new(1, sizeof(*self->pfds));

		if (!self->fds || !self->pfds) {
			gp_vec_free(self->fds);
			gp_vec_free(self->pfds);
			return 1;
		}
	} else {
		self->fds = gp_vec_append(self->fds, 1);
		self->pfds = gp_vec_append(self->pfds, 1);

		if (!self->fds || !self->pfds)
			return 1;
	}

	size_t last = gp_vec_len(self->fds) - 1;

	self->fds[last].event = event;
	self->fds[last].priv = priv;

	self->pfds[last].fd = fd;
	self->pfds[last].events = events;
	self->pfds[last].revents = 0;

	return 0;
}

static void rem(struct gp_fds *self, size_t pos)
{
	size_t len = gp_vec_len(self->fds);

	GP_DEBUG(2, "Removing fd %i event %p priv %p",
	         self->pfds[pos].fd, self->fds[pos].event, self->fds[pos].priv);

	self->fds[pos] = self->fds[len-1];
	self->pfds[pos] = self->pfds[len-1];

	self->fds = gp_vec_remove(self->fds, 1);
	self->pfds = gp_vec_remove(self->pfds, 1);
}

static ssize_t fd_to_pos(struct gp_fds *self, int fd)
{
	size_t i;

	for (i = 0; i < gp_vec_len(self->pfds); i++) {
		if (self->pfds[i].fd == fd)
			return i;
	}

	return -1;
}

int gp_fds_rem(struct gp_fds *self, int fd)
{
	ssize_t pos = fd_to_pos(self, fd);

	if (pos < 0)
		return 1;

	rem(self, pos);
	return 0;
}

int gp_fds_poll(struct gp_fds *self, int timeout)
{
	int ret;
	size_t i, len = gp_vec_len(self->pfds);

	ret = poll(self->pfds, len, timeout);
	if (ret <= 0)
		return ret;

	for (i = 0; i < len; i++) {
		if (self->pfds[i].revents) {
			ret = self->fds[i].event(&self->fds[i], &self->pfds[i]);
			self->pfds[i].revents = 0;
			if (ret) {
				rem(self, i);
				len--;
				i--;
			}
		}
	}

	return ret;
}
