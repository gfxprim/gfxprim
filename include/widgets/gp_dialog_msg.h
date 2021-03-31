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
	GP_DIALOG_MSG_QUESTION,
};

enum gp_dialog_retval {
	/** Generic exit -> no value reported */
	GP_DIALOG_EXIT = 1,
	/** Answer to a question dialog */
	GP_DIALOG_YES = 1,
	GP_DIALOG_NO = 2,
	/** Internal error */
	GP_DIALOG_ERR = 99,
};

/**
 * @brief Runs a dialog.
 *
 * @type A dialog type - enum gp_dialog_msg_type.
 * @title A dialog title.
 * @msg A dialog message.
 *
 * @return Returns enum gp_dialog_retval.
 */
int gp_dialog_msg_run(enum gp_dialog_msg_type type,
                      const char *title, const char *msg);

/**
 * @brief Runs a dialog.
 *
 * @type A dialog type - enum gp_dialog_msg_type.
 * @title A dialog title.
 * @fmt A printf-like format string for the dialog message.
 * @... A printf-like parameters.
 *
 * @return Returns enum gp_dialog_retval.
 */
int gp_dialog_msg_printf_run(enum gp_dialog_msg_type type, const char *title,
                             const char *fmt, ...)
                             __attribute__((format (printf, 3, 4)));

#endif /* GP_DIALOG_MESSAGE_H */
