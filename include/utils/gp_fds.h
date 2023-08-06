//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2023 Cyril Hrubis <metan@ucw.cz>

 */

/*
 * Simple wrapper around poll and gp_vec.
 */

#ifndef UTILS_GP_FDS__
#define UTILS_GP_FDS__

#include <utils/gp_vec.h>
#include <poll.h>

typedef struct gp_fds {
	struct gp_fd *fds;
	struct pollfd *pfds;
	int modified:1;
} gp_fds;

typedef struct gp_fd {
	/* Called when revent != 0 */
	int (*event)(struct gp_fd *self);
	struct pollfd *pfd;
	void *priv;
} gp_fd;

#define GP_FDS_INIT {}

/*
 * Removes all file descriptors.
 *
 * @self The fds struct.
 */
void gp_fds_clear(struct gp_fds *self);

/*
 * Adds a file descriptor.
 *
 * @self   The fds struct.
 * @fd     File descriptor to be added.
 * @events Event mask passed to poll().
 * @event  Event handler, called when poll revents are non-zero.
 * @priv   Private pointer passed to the event() handler.
 */
int gp_fds_add(struct gp_fds *self, int fd, short events,
               int (*event)(struct gp_fd *self), void *priv);

/*
 * Removes a file descriptor.
 *
 * @self The fds struct.
 * @fd   File descriptor to be removed.
 *
 * Returns 0 on success and -1 if fd is not in fds.
 */
int gp_fds_rem(struct gp_fds *self, int fd);

/*
 * Poll wrapper around the poll().
 *
 * Polls for file descriptors added by the gp_fds_add() function, calls event()
 * callback if events are returned.
 *
 * @self    The fds struct.
 * @timeout Timeout passed to poll().
 */
int gp_fds_poll(struct gp_fds *self, int timeout);

/*
 * @self The fds struct.
 * @return Number of fds in the queue.
 */
static inline size_t gp_fds_count(struct gp_fds *self)
{
	if (!self->fds)
		return 0;

	return gp_vec_len(self->fds);
}

#endif /* UTILS_GP_FDS__ */
