//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_SPINBUTTON_H__
#define GP_WIDGET_SPINBUTTON_H__

#include <widgets/gp_widget_choice.h>

/**
 * @brief Allocates and initializes new spinbutton widget.
 *
 * Spinbutton is a subclass of the choice widget.
 *
 * @choices An array of strings describing available choices.
 * @choice_cnt Size of the choices array.
 * @selected Initially selected choice.
 * @on_event An event handler.
 * @priv An user private pointer.
 *
 * @return A spinbutton widget.
 */
static inline gp_widget *gp_widget_spinbutton_new(const char *choices[],
                                                  unsigned int choice_cnt,
                                                  unsigned int selected,
                                                  int (*on_event)(gp_widget_event *self),
                                                  void *priv)
{
	return gp_widget_choice_new(GP_WIDGET_SPINBUTTON,
	                            choices, choice_cnt,
	                            selected, on_event, priv);
}

#endif /* GP_WIDGET_SPINBUTTON_H__ */
