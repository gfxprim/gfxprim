//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_BUTTON_H
#define GP_WIDGET_BUTTON_H

#include <widgets/gp_widget_bool.h>

enum gp_widget_button_type {
	GP_BUTTON_LABEL = 0,

	GP_BUTTON_OK,
	GP_BUTTON_CANCEL,
	GP_BUTTON_YES,
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
 * @brief Allocates a initialize new button widget.
 *
 * @label A button label.
 * @type A button type.
 *
 * @return A button widget.
 */
gp_widget *gp_widget_button_new(const char *label,
                                enum gp_widget_button_type type);


/**
 * @brief Allocates a initialize new button widget.
 *
 * @label A button label.
 * @type A button type.
 * @on_event An widget event handler.
 * @priv User private data.
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
 * @self A button widget.
 * @return A button type.
 */
enum gp_widget_button_type gp_widget_button_type_get(gp_widget *self);

/**
 * @brief Sets a button type.
 *
 * @self A button widget.
 * @type New button type.
 */
void gp_widget_button_type_set(gp_widget *self, enum gp_widget_button_type type);

#endif /* GP_WIDGET_BUTTON_H */
