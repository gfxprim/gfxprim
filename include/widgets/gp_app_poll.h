//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2025 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_poll.h
 * @brief Add file descriptors to the widget main loop.
 */

#ifndef WIDGETS_GP_APP_POLL_H
#define WIDGETS_GP_APP_POLL_H

#include <utils/gp_poll.h>

/**
 * @brief Inserts a file descriptor to the widget main loop.
 *
 * @param fd A poll fd structure.
 */
void gp_app_poll_add(gp_fd *fd);

/**
 * @brief Removes a file descriptor from the widget main loop.
 *
 * @param fd A poll fd structure.
 */
void gp_app_poll_rem(gp_fd *fd);

/**
 * @brief Looks up a fd struture by a file descriptor.
 *
 * @param fd A file descriptor.
 * @return A poll fd structure.
 */
gp_fd *gp_app_poll_rem_by_fd(int fd);

#endif /* WIDGETS_GP_APP_POLL_H */
