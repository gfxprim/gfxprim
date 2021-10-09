//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_DIALOG_INPUT_H
#define GP_DIALOG_INPUT_H

/**
 * @brief Runs an input dialog.
 *
 * @title A dialog title.
 * @msg A dialog message.
 *
 * @return Returns a new allocated string that should be freed with free().
 */
char *gp_dialog_input_run(const char *title);

#endif /* GP_DIALOG_INPUT_H */
