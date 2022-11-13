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

/*
 * Open file or director.
 */
enum gp_dialog_file_open_flags {
	GP_DIALOG_OPEN_FILE = 0x01,
	GP_DIALOG_OPEN_DIR = 0x02,
};

typedef struct gp_dialog_file_opts {
	/* used by file open dialog, do we want to open file/directory or both */
	enum gp_dialog_file_open_flags flags;
	/* hints for file extensions */
	const char *const *ext_hints;
} gp_dialog_file_opts;

/**
 * @brief Creates a file open dialog.
 *
 * @path An initial path for the dialog, set it to NULL for default.
 * @opts Additional options for the dialog.
 * @return Newly allocated dialog that can be executed by gp_dialog_run();
 */
gp_dialog *gp_dialog_file_open_new(const char *path,
                                   const gp_dialog_file_opts *opts);

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
                                   const gp_dialog_file_opts *opts);

#endif /* GP_WIDGET_DIALOG_FILE_H */
