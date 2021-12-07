//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_SPINBUTTON_H
#define GP_WIDGET_SPINBUTTON_H

#include <widgets/gp_widget_choice.h>

/**
 * @brief Allocates and initializes new spinbutton widget.
 *
 * Spinbutton is a subclass of the choice widget.
 *
 * @choices An array of strings describing available choices.
 * @cnt Size of the choices array.
 * @sel Initially selected choice.
 *
 * @return A spinbutton widget.
 */
static inline gp_widget *gp_widget_spinbutton_new(const char *choices[],
                                                  size_t cnt, size_t sel)
{
	return gp_widget_choice_new(GP_WIDGET_SPINBUTTON, choices, cnt, sel);
}

#endif /* GP_WIDGET_SPINBUTTON_H */
