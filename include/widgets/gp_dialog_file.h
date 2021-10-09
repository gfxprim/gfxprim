//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_DIALOG_FILE_H
#define GP_DIALOG_FILE_H

#include <widgets/gp_widget_types.h>

enum gp_dialog_file_open_exit {
	GP_WIDGET_DIALOG_PATH = 1,
	GP_WIDGET_DIALOG_CANCEL = 2,
};

/**
 * @brief Creates a file open dialog.
 *
 * @path An initial path for the dialog, set it to NULL for default.
 * @return Newly allocated dialog that can be executed by gp_dialog_run();
 */
gp_dialog *gp_dialog_file_open_new(const char *path);

/**
 * @brief Returns a file path for the file dalogs.
 *
 * @self A file open or file save dialog.
 * @return A string that is valid until dialog is destroyed.
 */
const char *gp_dialog_file_path(gp_dialog *self);

/**
 * @brief Creates a file save dialog.
 *
 * @path An initial path for the dialog, set it to NULL for default.
 * @return Newly allocated dialog that can be executed by gp_dialog_run();
 */
gp_dialog *gp_dialog_file_save_new(const char *path,
                                   const char *const ext_hints[]);

#endif /* GP_WIDGET_DIALOG_FILE_H */
