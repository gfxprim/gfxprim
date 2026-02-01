//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_dialog.h
 * @brief Implements dialog windows.
 */

#ifndef GP_DIALOG_H
#define GP_DIALOG_H

#include <core/gp_compiler.h>
#include <input/gp_types.h>
#include <widgets/gp_widget.h>
#include <widgets/gp_widget_types.h>

/**
 * @brief A dialog.
 *
 * A dialog is a widget layout that is temporarily shown on the top the current layout.
 */
struct gp_dialog {
	/** A wiget layout representing the dialog */
	gp_widget *layout;
	/** A dialog event handler input events not handled by widgets are passed here */
	int (*input_event)(gp_dialog *self, gp_event *ev);
	/** Set to non-zero to exit the dialog */
	long retval;

	char payload[] GP_ALIGNED;
};

/**
 * @brief Allocates and initializes new dialog.
 *
 * @param payload Payload size.
 * @return A newly allocated dialog.
 */
gp_dialog *gp_dialog_new(size_t payload);

/**
 * @brief Frees a dialog.
 *
 * If not NULL the the dialog layout is freed with the gp_widget_free() recursivelly.
 *
 * @param self A dialog.
 */
void gp_dialog_free(gp_dialog *self);

/**
 * @brief Loads a dialog layout given a dialog name.
 *
 * Looks for the layout in $HOME/.config/ and /etc/ if not found attempts to
 * parse layout from fallback JSON string.
 *
 * @param dialog_name Dialog name.
 * @param callbacks Structure with NULL terminated array of dialog callbacks.
 * @param fallback_json Fallback dialog JSON layout.
 * @param uids An pointer to store the has table UIDs to. Must be initialized to NULL.
 *
 * @return A widget layout or NULL in a case of a failure.
 */
gp_widget *gp_dialog_layout_load(const char *dialog_name,
                                 const gp_widget_json_callbacks *const callbacks,
                                 const char *fallback_json, gp_htable **uids);

#endif /* GP_DIALOG_H */
