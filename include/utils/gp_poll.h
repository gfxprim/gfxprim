//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2023 Cyril Hrubis <metan@ucw.cz>

 */

/*
 * Simple epoll wrapper.
 */

#ifndef UTILS_GP_POLL
#define UTILS_GP_POLL

#include <utils/gp_types.h>
#include <utils/gp_list.h>

/*
 * This maps 1:1 to epoll and poll
 */
enum gp_poll_events {
	GP_POLLIN = 0x01,
#define GP_POLLIN GP_POLLIN
	GP_POLLPRI = 0x02,
#define GP_POLLPRI GP_POLLPRI
	GP_POLLOUT = 0x04,
#define GP_POLLOUT GP_POLLOUT
	GP_POLLERR = 0x08,
#define GP_POLLERR GP_POLLERR
	GP_POLLHUP = 0x10,
#define GP_POLLHUP GP_POLLHUP
};

typedef struct gp_poll {
	gp_dlist fds;
	int ep_fd;
} gp_poll;

struct gp_fd {
	gp_dlist_head lhead;
	int (*event)(gp_fd *self);
	uint32_t events;
	uint32_t revents;
	int fd;
	void *priv;
};

/*
 * Removes all file descriptors.
 *
 * @self The gp_poll struct.
 */
void gp_poll_clear(gp_poll *self);

/*
 * Adds a file descriptor.
 *
 * @self   The gp_poll struct.
 * @fd     File descriptor to be added.
 * @events Event mask passed to poll().
 * @event  Event handler, called when poll revents are non-zero.
 * @priv   Private pointer passed to the event() handler.
 */
int gp_poll_add(gp_poll *self, gp_fd *fd);

/*
 * Removes a file descriptor.
 *
 * @self The fds struct.
 * @fd   File descriptor to be removed.
 *
 * Returns 0 on success and -1 if fd is not in fds.
 */
int gp_poll_rem(gp_poll *self, gp_fd *fd);

/*
 * Poll wrapper around the poll().
 *
 * Polls for file descriptors added by the gp_fds_add() function, calls event()
 * callback if events are returned.
 *
 * @self    The fds struct.
 * @timeout Timeout passed to poll().
 */
int gp_poll_wait(gp_poll *self, int timeout);

/*
 * @self The fds struct.
 * @return Number of fds in the queue.
 */
static inline size_t gp_poll_fds(gp_poll *self)
{
	return self->fds.cnt;
}

#endif /* UTILS_GP_POLL */
