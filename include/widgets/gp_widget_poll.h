//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef WIDGETS_GP_WIDGET_POLL_H
#define WIDGETS_GP_WIDGET_POLL_H

#include <utils/gp_poll.h>

/**
 * Inserts an fd into widgets file descriptors
 */
void gp_widget_poll_add(gp_fd *fd);

void gp_widget_poll_rem(gp_fd *fd);

gp_fd *gp_widget_poll_rem_by_fd(int fd);

#endif /* WIDGETS_GP_WIDGET_POLL_H */
