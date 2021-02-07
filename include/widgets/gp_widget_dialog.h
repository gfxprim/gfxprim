//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_DIALOG_H
#define GP_WIDGET_DIALOG_H

#include <widgets/gp_widget.h>
#include <widgets/gp_widget_types.h>

struct gp_widget_dialog {
	/** A wiget layout representing the dialog */
	gp_widget *layout;
	/** Set to non-zero on dialog exit, holds dialog return value */
	int dialog_exit;
	char payload[];
};

/**
 * @brief Allocates and initializes new dialog.
 *
 * @payload Payload size.
 * @return A newly allocated dialog.
 */
gp_widget_dialog *gp_widget_dialog_new(size_t payload);

/**
 * @brief Frees a dialog.
 *
 * If not NULL the the dialog layout is freed with the gp_widget_free() recursivelly.
 *
 * @self A dialog.
 */
void gp_widget_dialog_free(gp_widget_dialog *self);

/**
 * @brief Loads a dialog layout given a dialog name.
 *
 * Looks for the layout in $HOME/.config/ and /etc/ if not found attempts to
 * parse layout from fallback JSON string.
 *
 * @dialog_name Dialog name.
 * @fallback_json Fallback dialog JSON layout.
 * @uids An pointer to store the has table UIDs to. Must be initialized to NULL.
 *
 * @return A widget layout or NULL in a case of a failure.
 */
gp_widget *gp_dialog_layout_load(const char *dialog_name, const char *fallback_json, void **uids);

#endif /* GP_WIDGET_DIALOG_H */
