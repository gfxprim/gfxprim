//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_button.h
 * @brief A button widget.
 *
 * Button types
 * ------------
 *
 * @image html button_types.png
 *
 * Buttons with a specific type include a stock image in the rendered button,
 * they can have label as well but it's not required.
 *
 * @image html button_types_label.png
 *
 * Button widget JSON attributes
 * -----------------------------
 *
 * |   Attribute    |  Type  | Default |    Description    |
 * |----------------|--------|---------|-------------------|
 * |   **label**    | string |         | Button label      |
 * |   **btype**    | string | "label" | Button type       |
 * | **text_align** | string | depends | Button text align |
 *
 * The 'text_align' can be either 'left' or 'right' and the default value
 * depends on the 'btype'.
 */

#ifndef GP_WIDGET_BUTTON_H
#define GP_WIDGET_BUTTON_H

#include <widgets/gp_widget_class_bool.h>

/**
 * @brief A button type.
 */
enum gp_widget_button_type {
	/** @brief Button with a label without specific type. */
	GP_BUTTON_LABEL = 0,

	/** @brief An OK button. */
	GP_BUTTON_OK,
	/** @brief A Cancel button. */
	GP_BUTTON_CANCEL,
	/** @brief A Yes button. */
	GP_BUTTON_YES,
	/** @brief A No button. */
	GP_BUTTON_NO,

	/** @brief An open file or directory button. */
	GP_BUTTON_OPEN,
	/** @brief A save file button. */
	GP_BUTTON_SAVE,
	/** @brief A create new directory button. */
	GP_BUTTON_NEW_DIR,

	/** @brief A previous button. */
	GP_BUTTON_PREV,
	/** @brief A next button. */
	GP_BUTTON_NEXT,
	/** @brief A first button. */
	GP_BUTTON_FIRST,
	/** @brief A last button. */
	GP_BUTTON_LAST,
	/** @brief A top button. */
	GP_BUTTON_TOP,
	/** @brief A bottom button. */
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

/** @brief A gp_widget_event::sub_type for a button widget. */
enum gp_widget_button_event_type {
	/** @brief Emitted when button is pressed. */
	GP_WIDGET_BUTTON_TRIGGER,
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
