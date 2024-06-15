//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_dialog_msg.h
 * @brief A message dialogs.
 */

#ifndef GP_DIALOG_MSG_H
#define GP_DIALOG_MSG_H

#include <core/gp_compiler.h>
#include <widgets/gp_widget_types.h>

/**
 * @brief A message dialog type.
 */
enum gp_dialog_msg_type {
	/**
	 * @brief An info dialog.
	 *
         * @image html dialog_msg_info.png
	 */
	GP_DIALOG_MSG_INFO,
	/**
	 * @brief A warning dialog.
	 *
	 * @image html dialog_msg_warn.png
	 */
	GP_DIALOG_MSG_WARN,
	/**
	 * @brief An error dialog.
	 *
	 * @image html dialog_msg_err.png
	 */
	GP_DIALOG_MSG_ERR,
	/**
	 * @brief A yes or no question dialog.
	 *
	 * @image html dialog_question.png
	 */
	GP_DIALOG_MSG_QUESTION,
};

/**
 * @brief A dialog return value.
 */
enum gp_dialog_retval {
	/** Generic exit -> no value reported */
	GP_DIALOG_EXIT = 1,
	/** Answer to a question dialog */
	GP_DIALOG_YES = 1,
	/** Answer to a question dialog */
	GP_DIALOG_NO = 2,
	/** Internal error */
	GP_DIALOG_ERR = 99,
};

/**
 * @brief Runs a dialog.
 *
 * @param type A dialog type - enum gp_dialog_msg_type.
 * @param title A dialog title.
 * @param msg A dialog message.
 *
 * @return Returns enum gp_dialog_retval.
 */
int gp_dialog_msg_run(enum gp_dialog_msg_type type,
                      const char *title, const char *msg);

/**
 * @brief Runs a dialog.
 *
 * @param type A dialog type - enum gp_dialog_msg_type.
 * @param title A dialog title.
 * @param fmt A printf-like format string for the dialog message.
 * @param ... A printf-like parameters.
 *
 * @return Returns enum gp_dialog_retval.
 */
int gp_dialog_msg_printf_run(enum gp_dialog_msg_type type, const char *title,
                             const char *fmt, ...) GP_FMT_PRINTF(3, 4);

#endif /* GP_DIALOG_MSG_H */
