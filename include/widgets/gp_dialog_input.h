//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_dialog_input.h
 * @brief A text input dialog.
 */

#ifndef GP_DIALOG_INPUT_H
#define GP_DIALOG_INPUT_H

/**
 * @brief Runs an input dialog.
 *
 * @param title A dialog title.
 *
 * @return Returns a newly allocated string that should be freed with free().
 */
char *gp_dialog_input_run(const char *title);

#endif /* GP_DIALOG_INPUT_H */
