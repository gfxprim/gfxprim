//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_DIALOG_H
#define GP_DIALOG_H

#include <input/gp_types.h>
#include <widgets/gp_widget.h>
#include <widgets/gp_widget_types.h>

struct gp_dialog {
	/** A wiget layout representing the dialog */
	gp_widget *layout;
	/** A dialog event handler input events not handled by widgets are passed here */
	int (*input_event)(gp_dialog *self, gp_event *ev);
	/** Set to non-zero to exit the dialog */
	long retval;
	char payload[];
};

/**
 * @brief Allocates and initializes new dialog.
 *
 * @payload Payload size.
 * @return A newly allocated dialog.
 */
gp_dialog *gp_dialog_new(size_t payload);

/**
 * @brief Frees a dialog.
 *
 * If not NULL the the dialog layout is freed with the gp_widget_free() recursivelly.
 *
 * @self A dialog.
 */
void gp_dialog_free(gp_dialog *self);

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
gp_widget *gp_dialog_layout_load(const char *dialog_name, const char *fallback_json, gp_htable **uids);

#endif /* GP_DIALOG_H */
