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

gp_dialog *gp_dialog_file_open_new(const char *path);

const char *gp_dialog_file_open_path(gp_dialog *self);

#endif /* GP_WIDGET_DIALOG_FILE_OPEN_H */
