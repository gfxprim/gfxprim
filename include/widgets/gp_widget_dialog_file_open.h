//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_DIALOG_FILE_OPEN_H__
#define GP_WIDGET_DIALOG_FILE_OPEN_H__

#include <widgets/gp_widget_types.h>

enum gp_widget_dialog_file_open_exit {
	GP_WIDGET_DIALOG_PATH = 1,
	GP_WIDGET_DIALOG_CANCEL = 2,
};

gp_widget_dialog *gp_widget_dialog_file_open_new(const char *path);

const char *gp_widget_dialog_file_open_path(gp_widget_dialog *self);

#endif /* GP_WIDGET_DIALOG_FILE_OPEN_H__ */
