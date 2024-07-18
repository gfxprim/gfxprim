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
 * |   Attribute    |  Type  | Default | Description                                                    |
 * |----------------|--------|---------|----------------------------------------------------------------|
 * |   **label**    | string |         | A button label.                                                |
 * |   **btype**    | string | "label" | Button type, #gp_widget_button_type.                           |
 * | **text_align** | string | depends | Button text alignment, "left" or "right" from the stock image. |
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

	/** @brief A play button. */
	GP_BUTTON_PLAY,
	/** @brief A pause button. */
	GP_BUTTON_PAUSE,
	/** @brief A stop button. */
	GP_BUTTON_STOP,
	/** @brief A record button. */
	GP_BUTTON_REC,
	/** @brief A fast forward button. */
	GP_BUTTON_FFORWARD,
	/** @brief A fast backward button. */
	GP_BUTTON_FBACKWARD,
	/** @brief A step forward button. */
	GP_BUTTON_STEP_FORWARD,
	/** @brief A step backward button. */
	GP_BUTTON_STEP_BACKWARD,

	/** @brief A randomize button. */
	GP_BUTTON_RANDOMIZE,

	/** @brief An arrow up button. */
	GP_BUTTON_UP,
	/** @brief An arrow down button. */
	GP_BUTTON_DOWN,
	/** @brief An arrow left button. */
	GP_BUTTON_LEFT,
	/** @brief An arrow right button. */
	GP_BUTTON_RIGHT,

	/** @brief An add button. */
	GP_BUTTON_ADD,
	/** @brief A remove button. */
	GP_BUTTON_REM,

	/** @brief A clear button. */
	GP_BUTTON_CLEAR,
	/** @brief A backspace button. */
	GP_BUTTON_BACKSPACE,

	/** @brief A zoom in button. */
	GP_BUTTON_ZOOM_IN,
	/** @brief A zoom out button. */
	GP_BUTTON_ZOOM_OUT,
	/** @brief A zoom fit button. */
	GP_BUTTON_ZOOM_FIT,
	/** @brief A zoom normal button. */
	GP_BUTTON_ZOOM_NORMAL,

	/** @brief A rotate clockwise button. */
	GP_BUTTON_ROTATE_CW,
	/** @brief A rotate counter clockwise button. */
	GP_BUTTON_ROTATE_CCW,

	/** @brief A settings button. */
	GP_BUTTON_SETTINGS,
	/** @brief A home (directory) button. */
	GP_BUTTON_HOME,
	/** @brief A download button. */
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

/**
 * @brief Gets a button label.
 *
 * @param self A button widget.
 *
 * @return A button label or NULL if there is none.
 */
const char *gp_widget_button_label_get(gp_widget *self);

#endif /* GP_WIDGET_BUTTON_H */
