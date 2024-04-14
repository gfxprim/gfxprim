//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_button.h
 * @brief A button widget.
 */

#ifndef GP_WIDGET_BUTTON_H
#define GP_WIDGET_BUTTON_H

#include <widgets/gp_widget_bool.h>

/**
 * @brief A button type.
 *
 * @image html button_types.png
 *
 * Buttons with a specific type include a stock image in the rendered button,
 * they can have label as well but it's not required.
 *
 * @image html button_types_label.png
 */
enum gp_widget_button_type {
	/** Button just with a label */
	GP_BUTTON_LABEL = 0,

	/** An OK button */
	GP_BUTTON_OK,
	/** A Cancel button */
	GP_BUTTON_CANCEL,
	/** A Yes button */
	GP_BUTTON_YES,
	/** A No button */
	GP_BUTTON_NO,

	GP_BUTTON_OPEN,
	GP_BUTTON_SAVE,
	GP_BUTTON_NEW_DIR,

	GP_BUTTON_PREV,
	GP_BUTTON_NEXT,

	GP_BUTTON_FIRST,
	GP_BUTTON_LAST,
	GP_BUTTON_TOP,
	GP_BUTTON_BOTTOM,

	GP_BUTTON_PLAY,
	GP_BUTTON_PAUSE,
	GP_BUTTON_STOP,
	GP_BUTTON_REC,
	GP_BUTTON_FFORWARD,
	GP_BUTTON_FBACKWARD,
	GP_BUTTON_STEP_FORWARD,
	GP_BUTTON_STEP_BACKWARD,

	GP_BUTTON_RANDOMIZE,

	GP_BUTTON_UP,
	GP_BUTTON_DOWN,
	GP_BUTTON_LEFT,
	GP_BUTTON_RIGHT,

	GP_BUTTON_ADD,
	GP_BUTTON_REM,

	GP_BUTTON_CLEAR,
	GP_BUTTON_BACKSPACE,

	GP_BUTTON_ZOOM_IN,
	GP_BUTTON_ZOOM_OUT,
	GP_BUTTON_ZOOM_FIT,
	GP_BUTTON_ZOOM_NORMAL,

	GP_BUTTON_ROTATE_CW,
	GP_BUTTON_ROTATE_CCW,

	GP_BUTTON_SETTINGS,
	GP_BUTTON_HOME,
	GP_BUTTON_DOWNLOAD,

	GP_BUTTON_TYPE_MASK = 0x7fff,
	GP_BUTTON_ALIGN_MASK = 0x8000,
	GP_BUTTON_TEXT_LEFT = 0x8000,
	GP_BUTTON_TEXT_RIGHT = 0x0000,
};

/**
 * @brief Allocates and initializes a new button widget.
 *
 * @param label A button label.
 * @param type A button type.
 *
 * @return A button widget.
 */
gp_widget *gp_widget_button_new(const char *label,
                                enum gp_widget_button_type type);


/**
 * @brief Allocates a initialize new button widget.
 *
 * @param label A button label. Optional for buttons with type other than
 *              GP_BUTTON_LABEL.
 * @param type A button type.
 * @param on_event An widget event handler.
 * @param priv User private data.
 *
 * @return A button widget.
 */
static inline gp_widget *gp_widget_button_new2(const char *label,
                                               enum gp_widget_button_type type,
                                               int (*on_event)(gp_widget_event *ev),
                                               void *priv)
{
	gp_widget *ret = gp_widget_button_new(label, type);

	gp_widget_on_event_set(ret, on_event, priv);

	return ret;
}

/**
 * @brief Returns a button type.
 *
 * The aligment is masked out of the return value.
 *
 * @param self A button widget.
 * @return A button type.
 */
enum gp_widget_button_type gp_widget_button_type_get(gp_widget *self);

/**
 * @brief Sets a button type.
 *
 * @param self A button widget.
 * @param type New button type.
 */
void gp_widget_button_type_set(gp_widget *self, enum gp_widget_button_type type);

#endif /* GP_WIDGET_BUTTON_H */
