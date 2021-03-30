//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_DIALOG_MESSAGE_H
#define GP_DIALOG_MESSAGE_H

#include <widgets/gp_widget_types.h>

enum gp_dialog_msg_type {
	GP_DIALOG_MSG_INFO,
	GP_DIALOG_MSG_WARN,
	GP_DIALOG_MSG_ERR,
};

enum gp_dialog_exit {
	/* generic exit -> no value reported */
	GP_DIALOG_EXIT = 1,
	/* internal error */
	GP_DIALOG_ERR = 99,
};

gp_dialog *gp_dialog_msg_new(enum gp_dialog_msg_type type, const char *msg);

int gp_dialog_msg_run(enum gp_dialog_msg_type type, const char *msg);

int gp_dialog_msg_printf_run(enum gp_dialog_msg_type type, const char *fmt, ...)
                             __attribute__((format (printf, 2, 3)));

#endif /* GP_DIALOG_MESSAGE_H */
