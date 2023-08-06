//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef WIDGETS_GP_WIDGET_FDS_H
#define WIDGETS_GP_WIDGET_FDS_H

#include <utils/gp_fds.h>

/**
 * Inserts an fd into widgets file descriptors
 */
void gp_widget_fds_add(int fd, short events, int (*event)(gp_fd *self), void *priv);

void gp_widget_fds_rem(int fd);

#endif /* WIDGETS_GP_WIDGET_FDS_H */
