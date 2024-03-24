//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2023 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_poll.h
 * @brief A simple epoll wrapper.
 */

#ifndef UTILS_GP_POLL
#define UTILS_GP_POLL

#include <utils/gp_types.h>
#include <utils/gp_list.h>

/**
 * @brief A gp_epoll flags.
 *
 * This maps 1:1 to epoll and poll
 */
enum gp_poll_events {
	/** There are data to read */
	GP_POLLIN = 0x01,
	/** There is exceptional condition on the fd */
	GP_POLLPRI = 0x02,
	/** Writing is now possible */
	GP_POLLOUT = 0x04,
	/** Error condition has happended */
	GP_POLLERR = 0x08,
	/** Hang up */
	GP_POLLHUP = 0x10,
};

#define GP_POLLIN GP_POLLIN
#define GP_POLLPRI GP_POLLPRI
#define GP_POLLOUT GP_POLLOUT
#define GP_POLLERR GP_POLLERR
#define GP_POLLHUP GP_POLLHUP

/**
 * @brief An epoll instance.
 */
typedef struct gp_poll {
	/** A double linked list of struct gp_fd */
	gp_dlist fds;
	/** An epoll file decriptor */
	int ep_fd;
} gp_poll;

/**
 * @brief A return value from the event() callback.
 */
enum gp_poll_event_ret {
	/** Processing datat was fine */
	GP_POLL_RET_OK = 0,
	/** Remove the fd from the epoll list */
	GP_POLL_RET_REM = 1,
};

/**
 * @brief An epoll file descriptor.
 *
 * The user of this interface is supposed to set the event, events, fd and
 * optionally priv pointer and pass the structure to the gp_poll_add()
 * function.
 */
struct gp_fd {
	/** @brief Linked list pointers */
	gp_dlist_head lhead;
	/**
	 * @brief Epoll event handler.
	 *
	 * This callback is called for events on the fd.
	 */
	enum gp_poll_event_ret (*event)(gp_fd *self);
	/** @brief Epoll events to watch. */
	uint32_t events;
	/** @brief Events returned from epoll */
	uint32_t revents;
	/** @brief A file descriptor. */
	int fd;
	/** User private pointer, not used by the library */
	void *priv;
};

/**
 * @brief Removes all file descriptors from the poll.
 *
 * @param self The gp_poll struct.
 */
void gp_poll_clear(gp_poll *self);

/**
 * @brief Adds a file descriptor.
 *
 * @param self The gp_poll struct.
 * @param fd A struct gp_fd filled in by the user.
 *
 * @return 0 on success and -1 if the underlying epoll() call failed.
 */
int gp_poll_add(gp_poll *self, gp_fd *fd);

/**
 * @brief Removes a file descriptor.
 *
 * @param self The poll struct.
 * @param fd File descriptor to be removed.
 *
 * @return 0 on success and -1 if fd is not in fds.
 */
int gp_poll_rem(gp_poll *self, gp_fd *fd);

/**
 * @brief Looks up and removes a gp_fd by a fd.
 *
 * @param self The poll struct.
 * @param fd A file descriptor for the lookup.
 *
 * @return A gp_fd if found or NULL.
 */
gp_fd *gp_poll_rem_by_fd(gp_poll *self, int fd);

/**
 * @brief A wrapper around the poll().
 *
 * Polls for file descriptors added by the gp_fds_add() function, calls event()
 * callback for each fd if events are returned.
 *
 * @param self The fds struct.
 * @param timeout_ms Timeout passed to poll().
 * @return A zero on success, -1 otherwise.
 */
int gp_poll_wait(gp_poll *self, int timeout_ms);

/**
 * @brief Returns a number of fds in the poll instance.
 *
 * @param self The fds struct.
 * @return Number of fds in the queue.
 */
static inline size_t gp_poll_fds(gp_poll *self)
{
	return self->fds.cnt;
}

#endif /* UTILS_GP_POLL */
